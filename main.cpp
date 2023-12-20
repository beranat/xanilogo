// Copyright (C) 2017 A. madRat L. Berenblit <beranat@users.noreply.github.com>
//------------------------------------------------------------------------------
// This file is part of XAniLogo application.
//------------------------------------------------------------------------------
// XAniLogo is  free software:  you can redistribute it  and/or modify it  under
// the terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option)
// any later version.
//------------------------------------------------------------------------------
// XAniLogo distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//------------------------------------------------------------------------------
// You should have received a copy of the GNU General Public License along with
// application. If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------------
// vim: noexpandtab shiftwidth=4 tabstop=4 :

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdexcept>
#include <memory>
#include <iostream>
#include <iterator>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <limits>

#define _USE_MATH_DEFINES
#include <cmath>

#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/limits.h>

#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include "main.h"
#include "xparam.h"
#include "xostream.h"
#include "matrix.hpp"
#include "xlogo.xbm"

typedef std::shared_ptr<_XrmHashBucketRec> XrmDatabasePtr;
typedef Matrix4x4<float> FloatMatrix;
typedef Vector4<float> FloatVector;

static const char *VERSION = "1.0.0";
static const char *const APP_NAME = "xanilogo";

std::shared_ptr<Display> appDisplay(nullptr);

// Arguments
static bool isMultipleAccess = false;
static bool isDoubleBuffer = false;
static int maxFps = 30;
static bool isShowFps = false;
std::fstream logFile;
static XRectangle geom = {80, 60, 480, 360};

static int animationPeriod  = 800;
static int visiblePeriod    = 500;
static int hiddenPeriod     = 400;

static const float maxScale = 0.5f;     // 1/2 of screen for largest X
static const float minScale = 0.25f;    // 1/4 of screen for smallest X

volatile sig_atomic_t isStopping = 0;

static Window windowId = None;
static XColor colorBlack = {0, 0x0000, 0x0000, 0x0000};
static XColor colorWhite = {0, 0xFFFF, 0xFFFF, 0xFFFF};

static XColor colorXobject = {0, 0x8000, 0x8000, 0x8000};
static bool isRandom = true;

const unsigned long long fpsPeriod = 500; // re-calc fps every 1/2 second
unsigned long long fpsStart = 0;
unsigned long long fpsCount = 0;
float fpsValue = -1.0f;

unsigned long long animStart = 0;
FloatVector animAxis(0, 1, 0, 0);
FloatVector animPos(0, 0, 0, 1);
float animScale = 0.75f;

FloatMatrix viewport(1);
const FloatMatrix projection = setOrthographic<float>() // setPerspective(3.0f, 10.0f)
							   *setScale(FloatVector(-0.75f, -0.75f, 1))
							   *setTranslation(FloatVector(0.f, 0.f, 2.0f));
// X-logo data
static const XColor Xcolors[] = {
	{0, 0x8000, 0x8000, 0x8000},	// white
	{0, 0x8000, 0x8000, 0x0000},	// yellow
	{0, 0x8000, 0x0000, 0x8000},	// magenta
	{0, 0x8000, 0x0000, 0x0000},	// red
	{0, 0x0000, 0x8000, 0x8000},	// cyan
	{0, 0x0000, 0x8000, 0x0000},	// green
	{0, 0x0000, 0x0000, 0x8000},	// blue
};

static const XPoint Xsize = {273, 273};
static const XPoint Xdata[] = {0, 273, 104, 140, 0, 0, 67, 0, 151, 113, 24, 273};

// Arguments
static const char *argShortHelp = "-h";
static const char *argLongHelp = "--help";

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
static const XrmOptionDescRec xrmOptions[] {
	{"-geom",       ".geometry",    XrmoptionSepArg, NULL},
	{"-double",     ".doublebuffer", XrmoptionNoArg,  "true"},
	{"-shared",     ".shared",      XrmoptionNoArg,  "true"},
	{"-color",      ".color",       XrmoptionSepArg, NULL},
	{"-anim",       ".animation",   XrmoptionSepArg, NULL},
	{"-visible",    ".visible",     XrmoptionSepArg, NULL},
	{"-hidden",     ".hidden",      XrmoptionSepArg, NULL},
	{"-showfps",    ".showFPS",     XrmoptionNoArg,  "true"},
	{"-maxfps",     ".maxFPS",      XrmoptionSepArg, NULL},
	{"-log",        ".logFile",     XrmoptionSepArg, NULL},
	{"-root",       ".root",        XrmoptionNoArg,  "true"},
	{"-window-id",  ".windowId",    XrmoptionSepArg, NULL}
};
#pragma GCC diagnostic pop

char const *const getAppName() {
	return APP_NAME;
}

Display *getAppDisplay() {
	if (!appDisplay)
		throw std::runtime_error("Display pointer is not available");
	return appDisplay.get();
}

int xErrorHandler(Display *display, XErrorEvent *event) {
	char desc[128] = { 0 };
	XGetErrorText(display, event->error_code, desc, sizeof(desc) / sizeof(*desc) - 1);
	std::cerr<<"X11 Error: "<<desc<<" (code="<<static_cast<int>(event->error_code)<<")."<<std::endl;
	isStopping = true;
	return 0;
}

void breakSignalHandler(int signal) {
	isStopping = true;
	static const char *const message = "Process break signal received\n";
	write(STDOUT_FILENO, message, strlen(message));
}

unsigned long long getNow() {
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	return (tv.tv_sec*1000ULL)+(tv.tv_usec/1000ULL);
}

float getRandom(float min=-1.0f, float max=1.0f) {
	return min + rand()*(max-min)/RAND_MAX;
}

typedef std::shared_ptr<unsigned long> XColorPtr;

void replaceColor(const Colormap &colormap, const XColor &color, XColorPtr &handle) {
	handle.reset();
	XColor c = color;
	if (0 == XAllocColor(getAppDisplay(), colormap, &c)) {
		std::cerr<<"WARN: Can not allocate color, use global-white."<<std::endl;
		handle.reset(new unsigned long(colorWhite.pixel));
	} else {
		handle.reset(new unsigned long(c.pixel),
		[colormap](unsigned long *p) {
			XFreeColors(getAppDisplay(), colormap, p, 1, 0);
			delete p;
		});
	}
}

void renderX(const Drawable &window, const GC &gc, const Colormap &colormap, XColorPtr &color,
			 const float &sx, const float &sy) {

	long long pos = getNow() - animStart;

	if ((pos < 0) || (pos >= (animationPeriod<<1)+visiblePeriod+hiddenPeriod)) {
		animStart += pos;
		pos = 0;

		animAxis[0] = getRandom();
		animAxis[1] = getRandom();
		animAxis[2] = 0.0f;

		animPos[0] = sx*getRandom();
		animPos[1] = sy*getRandom();
		animPos[2] = 0.0f;

		animScale = getRandom(minScale, maxScale);

		if (isRandom)
			replaceColor(colormap, Xcolors[rand() % (sizeof(Xcolors)/sizeof(*Xcolors))], color);
	}

	FloatMatrix model =    setTranslation(FloatVector(-1, -1))*
						   setScale(FloatVector(2.0f/Xsize.x, 2.0f/Xsize.y));
	FloatMatrix rotate= setRotation((float)M_PI, FloatVector(0, 0, 1, 0));

	pos -= animationPeriod;

	if (pos >= (animationPeriod+visiblePeriod))
		return;

	if (pos > 0)
		pos = (pos < visiblePeriod)?0:(pos-visiblePeriod);

	const FloatMatrix anim = setRotation((float)(pos*M_PI/2.0f/animationPeriod), animAxis);

	FloatVector normal(0, 0, 1, 0);
	FloatVector eye(0, 0, 1, 0);
	if (eye*(anim*normal) < std::numeric_limits<float>::epsilon())
		return;

	XSetForeground(getAppDisplay(), gc, *color);

	std::vector<XPoint> g0(std::begin(Xdata), std::end(Xdata));
	std::vector<XPoint> g1;
	g1.reserve(g0.size());

	const FloatMatrix panim = projection
							  *setTranslation(animPos)
							  *setScale(FloatVector(animScale, animScale, animScale))
							  *anim;
	for (auto &g : g0) {
		const FloatVector p = model*FloatVector(g);
		g1.push_back(viewport*~(panim*rotate*p));
		g = viewport*~(panim*p);
	};

	XFillPolygon(getAppDisplay(), window, gc, &*g0.begin(), g0.size(), Nonconvex, CoordModeOrigin);
	XFillPolygon(getAppDisplay(), window, gc, &*g1.begin(), g1.size(), Nonconvex, CoordModeOrigin);
}

typedef std::unique_ptr<Pixmap, void(*)(Pixmap *)> PixmapHandle;

void releasePixmap(Pixmap *p) {
	if (nullptr == p) {
		std::cerr<<"WARN: Invalid pointer to pixmap"<<std::endl;
		return;
	}
	if (None != *p)
		XFreePixmap(getAppDisplay(), *p);
	delete p;
}

void render(const Window &window, const GC &gc, const Colormap &colormap, PixmapHandle &doubleBuffer,
			XColorPtr &color) {
	{
		// FPS calculate
		++fpsCount;
		const auto diff = getNow() - fpsStart;

		if (diff > fpsPeriod) {
			fpsValue = fpsCount*1.0f*fpsPeriod / diff;
			fpsStart += diff;
			fpsCount = 0;
		}
	}

	XWindowAttributes geometry;
	XGetWindowAttributes(getAppDisplay(), window, &geometry);
	geometry.x = 0;
	geometry.y = 0;

	Drawable drawable = window;
	if (isDoubleBuffer) {
		int x, y;
		unsigned int width = 0, height = 0, border, depth = 0;
		Window root;

		if (!!doubleBuffer && None != *doubleBuffer)
			XGetGeometry(getAppDisplay(), *doubleBuffer, &root, &x, &y, &width, &height, &border, &depth);

		if (geometry.width != width || geometry.height != height || geometry.depth != depth) {
			doubleBuffer.reset(
				new Pixmap(
					XCreatePixmap(getAppDisplay(), window, geometry.width, geometry.height, geometry.depth)));
		}
		drawable = *doubleBuffer;
	}

	XSetForeground(getAppDisplay(), gc, colorBlack.pixel);
	XFillRectangle(getAppDisplay(), drawable, gc, geometry.x, geometry.y, geometry.width, geometry.height);


	const int length = std::min(geometry.width, geometry.height);
	viewport = setViewport<float>(geometry.x+(geometry.width-length)/2,
								  geometry.y+(geometry.height-length)/2,
								  length,	length);

	renderX(drawable, gc, colormap, color,
			static_cast<float>(geometry.width)/length, static_cast<float>(geometry.height)/length);

	if (isShowFps) {
		XSetForeground(getAppDisplay(), gc, colorWhite.pixel);
		char message[128];
		sprintf(message, "%.2f fps", fpsValue);
		XCharStruct info;
		int dir, asc, des;
		XQueryTextExtents(getAppDisplay(), XGContextFromGC(gc), message, strlen(message), &dir, &asc, &des, &info);
		XDrawString(getAppDisplay(), drawable, gc,
					geometry.x+geometry.width-info.width-asc-des, geometry.y+asc+des,
					message, strlen(message));
	}

	if (isDoubleBuffer)
		XCopyArea(getAppDisplay(), drawable, window, gc,
				  0, 0, geometry.width, geometry.height, geometry.x, geometry.y);
}

template<class T> bool getValue(XrmDatabasePtr &db, const char *name, T &value) {
	return getValue(db.get(), name, value);
}

template<class T, class D> bool getValue(XrmDatabasePtr &db, const char *name, T &value, const D def) {
	return getValue(db.get(), name, value, def);
}

void initVariables(int argc, char *argv[]) {
	XrmInitialize();

	const char* manager = XResourceManagerString(getAppDisplay());
	XrmDatabase tmp = XrmGetStringDatabase(manager);

	XrmParseCommand(&tmp, const_cast<XrmOptionDescRec *>(xrmOptions),
					sizeof(xrmOptions)/sizeof(*xrmOptions), APP_NAME, &argc, argv);
	XrmDatabasePtr db(tmp, XrmDestroyDatabase);
	getValue(db, "doublebuffer", isDoubleBuffer);
	getValue(db, "shared",		isMultipleAccess);
	isRandom = !getValue(db, "color", colorXobject);

	getValue(db, "geometry",	geom);
	getValue(db, "animation",   animationPeriod, animationPeriod);
	getValue(db, "visible",     visiblePeriod, visiblePeriod);
	getValue(db, "hidden",      hiddenPeriod, hiddenPeriod);
	getValue(db, "showFPS",     isShowFps, isShowFps);
	getValue(db, "maxFPS",      maxFps, maxFps);

	if (std::string logName; getValue(db, "logFile", logName) && !logName.empty()) {
		logFile.open(logName, std::fstream::out);

		if (logFile.is_open()) {
			std::cerr.rdbuf(logFile.rdbuf());
			std::cerr.tie(&logFile);
		} else
			std::cerr<<"WARN: log-file "<<logName<<" openning error, stderr will be used."<<std::endl;
	}

	if (bool isRoot = false; getValue(db, "root", isRoot) && isRoot)
		windowId = XDefaultRootWindow(getAppDisplay());

	if (Window wid = None; getValue(db, "windowId", wid))
		windowId = wid;
	else if (const char *xid = getenv("XSCREENSAVER_WINDOW"); xid != NULL) {
		char *tail = nullptr;
		const Window id = static_cast<Window>(strtoul(xid, &tail, 0));
		if ((errno == ERANGE) || (nullptr == tail) || (0 != *tail && !isspace(*tail)))
			throw std::runtime_error("env/XSCREENSAVER_WINDOW invalid windowId " + std::string(xid));

		windowId = id;
	}

	if (windowId != None) {
		XWindowAttributes geometry;
		if (0 == XGetWindowAttributes(getAppDisplay(), windowId, &geometry))
			throw std::runtime_error("Specified invalid or restricted windowId");
	}
}

std::string getProcessPath(const char *arg0) {

	{
		// QUERY 'PROC_SELF'
		static constexpr const char *PROC_SELF = "/proc/self/exe";
		std::string path(std::max(PATH_MAX, 0x100), '\0');
		do {
			const ssize_t r = readlink(PROC_SELF, &*path.begin(), path.size());
			if (-1 == r) {
				const int error = errno;
				std::cerr<<"NOTICE: "<<"Read process link '"<<PROC_SELF<<"' errno="<<error<<", skipped."<<std::endl;
				break;
			}

			if (path.size() > r)
				return path;

			path.resize(path.size()<<1);
		} while (true);
	}

	char *path = realpath(arg0, NULL);
	if (nullptr == path)
		throw std::system_error(errno, std::system_category(), "No realpath for argv[0]");

	std::string result = path;
	free(path);
	path = nullptr;

	if (result.length() < 2) // "/a" - smallest valid name
		throw std::runtime_error("Invalid realpath for argv");

	return path;
}

int main(int argc, char *argv[]) try {
	if (2 == argc && (0 == strcmp(argShortHelp, argv[1]) || 0 == strcmp(argLongHelp,  argv[1]))) {
		std::cout<<APP_NAME<<" version "<<VERSION<<" Copyright (c) 2002 by madRat"<<std::endl
				 <<std::endl
				 <<"Usage: "<<argv[0]<<" "<<argShortHelp<<" ("<<argLongHelp<<")";

		for (const auto &v : xrmOptions) {
			std::cout<<" | "<<v.option;

			switch (v.argKind) {
				case XrmoptionStickyArg:
					std::cout<<"VALUE";
					break;
				case XrmoptionSepArg:
					std::cout<<" value";
					break;
			}
		}
		std::cout<<std::endl;
		return EXIT_SUCCESS;
	}

	std::shared_ptr<int> sharedAccess;

	srand(time(NULL));
	appDisplay.reset(XOpenDisplay(NULL), XCloseDisplay);
	if (!appDisplay)
		throw std::runtime_error("Can not open getAppDisplay()");

	XSetErrorHandler(xErrorHandler);
	if (SIG_ERR == signal(SIGTERM, breakSignalHandler))
		throw std::system_error(errno,  std::system_category(), "set signal handler");

	initVariables(argc, argv);

	const int screen = XDefaultScreen(getAppDisplay());
	PixmapHandle icon(nullptr, releasePixmap);
	std::shared_ptr<Window> window(new Window(windowId));

	if (None == *window) {
		if (isMultipleAccess)
			std::cerr<<"NOTICE: Multiple access is not applicable to the private window, ignored."<<std::endl;

		Window wnd = XCreateSimpleWindow(getAppDisplay(), XDefaultRootWindow(getAppDisplay()),
											geom.x, geom.y, geom.width, geom.height, 1,
											XWhitePixel(getAppDisplay(), screen),
											XBlackPixel(getAppDisplay(), screen));
		if (None == wnd)
			throw std::runtime_error("Can not create main window");
			window.reset(new Window(wnd), [](Window *w) {
				XDestroyWindow(getAppDisplay(), *w);
				delete w;
			});

		XStoreName(getAppDisplay(), wnd, APP_NAME);
		XSetIconName(getAppDisplay(), wnd, APP_NAME);

		XSizeHints sh = { 0 };
		sh.width = sh.min_width = geom.width;
		sh.height = sh.min_height = geom.height;
		sh.x = geom.x;
		sh.y = geom.y;
		sh.flags = PSize | PMinSize | PPosition;
		XSetWMNormalHints(getAppDisplay(), wnd, &sh);

		icon.reset(new Pixmap(
					   XCreateBitmapFromData(getAppDisplay(), wnd, reinterpret_cast<const char *>(xlogo_bits),
											 xlogo_width, xlogo_height)));
		XWMHints wh = { 0 };
		wh.flags = IconPixmapHint;
		wh.icon_pixmap = *icon;
		XSetWMHints(getAppDisplay(), wnd, &wh);

		Atom delWindow = XInternAtom(getAppDisplay(), "WM_DELETE_WINDOW", False);
		XSetWMProtocols(getAppDisplay(), wnd, &delWindow, 1);

		XSelectInput(getAppDisplay(), wnd, ExposureMask | KeyPressMask | ButtonPressMask);

		XMapWindow(getAppDisplay(), wnd);
	} else {
		if (isMultipleAccess) {
			// Simple lock on exe file - need not extra file(s)
			{
				const int fd = open(getProcessPath(argv[0]).c_str(), O_CLOEXEC | O_RDONLY, 0);
				if (fd < 0)
					throw std::system_error(errno, std::system_category(), "open exec file");
				sharedAccess.reset(new int(fd), [](int *p) {
					close(*p);
					delete p;
				});
			}

			bool isShownInNotice = false;
			while (!isStopping) {
				if (0 <= flock(*sharedAccess, LOCK_EX | LOCK_NB))
					break;

				if (EAGAIN != errno && EINTR != errno)
					throw std::system_error(errno, std::system_category(), "flock exec file");

				if (!isShownInNotice) {
					std::cerr<<"NOTICE: Multiple access collision detected."<<std::endl;
					isShownInNotice = true;
				}

				usleep(250000);
			}

			if (isStopping)
				return EXIT_SUCCESS;

			if (isShownInNotice)
				std::cerr<<"NOTICE: Multiple access lock owned."<<std::endl;
		}
	}

	Colormap colormap = XDefaultColormap(getAppDisplay(), screen);
	colorWhite.pixel = XWhitePixel(getAppDisplay(), screen);
	colorBlack.pixel = XBlackPixel(getAppDisplay(), screen);
	XColorPtr color;
	replaceColor(colormap, colorXobject, color);

	XGCValues gcValues;
	gcValues.function           = GXcopy;
	gcValues.plane_mask         = AllPlanes;
	gcValues.foreground         = colorWhite.pixel;
	gcValues.background         = colorBlack.pixel;
	gcValues.line_width         = 0;
	gcValues.line_style         = LineSolid;
	gcValues.cap_style          = CapButt;
	gcValues.join_style         = JoinMiter;
	gcValues.fill_style         = FillSolid;
	gcValues.fill_rule          = WindingRule;
	gcValues.graphics_exposures = False;
	gcValues.clip_x_origin      = 0;
	gcValues.clip_y_origin      = 0;
	gcValues.clip_mask          = None;
	gcValues.subwindow_mode     = IncludeInferiors;

	std::shared_ptr<_XGC> gc(XCreateGC(getAppDisplay(), *window,
									   GCFunction | GCPlaneMask
									   | GCForeground | GCBackground
									   | GCLineWidth | GCLineStyle
									   | GCCapStyle | GCJoinStyle
									   | GCFillStyle | GCFillRule
									   | GCGraphicsExposures
									   | GCClipXOrigin | GCClipYOrigin | GCClipMask
									   | GCSubwindowMode, &gcValues),
	[](GC c) {
		XFreeGC(getAppDisplay(), c);
	});

	PixmapHandle doubleBuffer(nullptr, releasePixmap);

	unsigned long long refresh = 0;
	while (!isStopping) {
		if (0 < XPending(getAppDisplay())) {
			XEvent e;
			XNextEvent(getAppDisplay(), &e);

			switch (e.type) {
				case KeyPress:
				case ButtonPress:
				case ClientMessage:
					isStopping = 1;
					break;
				case Expose:
					render(*window, gc.get(), colormap, doubleBuffer, color);
			}
		} else {
			const auto diff = getNow() - refresh;
			if (maxFps > 0) {
				const long long remain = (1000ULL / maxFps) - diff;

				if (remain < 0) {
					render(*window, gc.get(), colormap, doubleBuffer, color);
					XSync(getAppDisplay(), False);
					refresh += diff;
				} else
					usleep(remain*1000ULL);
			}
		}
	} // while (!isStopping)
	return EXIT_SUCCESS;
} catch (const std::exception &e) {
	const char *msg = e.what();
	if (nullptr == msg || 0 == *msg)
		msg = "std::exception w/o message";
	std::cerr<<"ERROR: "<<e.what()<<(('.'!=msg[strlen(msg)-1])?".":"")<<std::endl;
	return EXIT_FAILURE;
}
