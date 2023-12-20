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

#include <string.h>
#include <stdio.h>

#include <iostream>
#include <sstream>

#include "main.h"
#include "xostream.h"
#include "xparam.h"

bool isStartWith(const std::string &s, const char *prefix) {
	return 0 == strncasecmp(s.c_str(), prefix, strlen(prefix));
}

bool getValue(XrmDatabase db,
			  const char *name,
			  std::string &value,
			  const char *def,
			  const char *className) {

	char *type = nullptr;
	std::ostringstream s;
	s<<getAppName()<<"."<<name;
	const std::string fullName = s.str();

	XrmValue ret;
	do {
		if (!XrmGetResource(db,
							fullName.c_str(),
							(nullptr != className && 0 != *className)?className:"String",
							&type,
							&ret))
			break;

		if (0 != strcmp(type, "String")) {
			std::cerr<<"WARN: Resource "<<fullName<<" has unsupported type "<<type<<", skipped."<<std::endl;
			break;
		}
		value.assign(reinterpret_cast<const char *>(ret.addr),
					 reinterpret_cast<const char *>(ret.addr)+ret.size);
		return true;
	} while (false);

	if (nullptr != def) {
		value = def;
		return true;
	}
	return false;
}

bool getValue(XrmDatabase db,
			  const char *name,
			  std::string &value,
			  const char *def) {
	return getValue(db, name, value, def, nullptr);
}

bool getValue(XrmDatabase db, const char *name, int &value, int def) {
	std::string v;
	if (!getValue(db, name, v, nullptr, "Integer")) {
		value = def;
		return true;
	}

	const char *format = "%d";
	if (isStartWith(v, "0x"))
		format = "0x%x";

	if (1 != sscanf(v.c_str(), format, &value)) {
		std::cerr<<"WARN: Resource "<<name<<" value '"<<v<<"' can not be integer, skipped."<<std::endl;
		return false;
	}
	return true;
}

bool getValue(XrmDatabase db, const char *name, XRectangle &value) {
	std::string v;
	if (!getValue(db, name, v, nullptr, "Geometry"))
		return false;

	v.push_back(0);
	int x, y;
	unsigned int cx, cy;
	if (0 == XParseGeometry(&*v.begin(), &x, &y, &cx, &cy)) {
		std::cerr<<"WARN: Resource "<<name<<" value '"<<v<<"' can not be x-geometry, skipped."<<std::endl;
		return false;
	}
	value.x = static_cast<short>(x);
	value.y = static_cast<short>(y);
	value.width = static_cast<short>(cx);
	value.height = static_cast<short>(cy);
	return true;
}

bool getValue(XrmDatabase db, const char *name, XID &value) {
	std::string v;
	if (!getValue(db, name, v, nullptr, "XID"))
		return false;

	if (0 == strcasecmp("none", v.c_str())) {
		value = None;
		return true;
	}

	const char *format = "%lu";
	if (isStartWith(v, "0x"))
		format = "0x%lx";

	unsigned long int xid;
	if (1 != sscanf(v.c_str(), format, &xid)) {
		std::cerr<<"WARN: Resource "<<name<<" value '"<<v<<"' can not be XID, skipped."<<std::endl;
		return false;
	}
	value = xid;
	return true;
}

bool getValue(XrmDatabase db, const char *name, XColor &value) {

	std::string v;
	if (!getValue(db, name, v, nullptr, "Color"))
		return false;

	v.push_back(0);
	XColor color;
	Display *const display = getAppDisplay();
	const int screen = XDefaultScreen(display);
	Colormap map = XDefaultColormap(display, screen);
	if (0 == XParseColor(display, map, &*v.begin(), &color)) {
		std::cerr<<"WARN: Resource "<<name<<" value '"<<v<<"' can not be XColor, skipped."<<std::endl;
		return false;
	}
	value = color;
	return true;
}

bool getValue(XrmDatabase db, const char *name, bool &value, bool def) {
	std::string v;
	if (!getValue(db, name, v, (def)?"true":"false", "Boolean"))
		return false;

	if ((0 == strcasecmp("true", v.c_str()))
			|| (0 == strcasecmp("on", v.c_str()))
			|| (0 == strcasecmp("yes", v.c_str()))
			|| (0 == strcasecmp("1", v.c_str())))
		value = true;

	else if ((0 == strcasecmp("false", v.c_str()))
			 || (0 == strcasecmp("off", v.c_str()))
			 || (0 == strcasecmp("no", v.c_str()))
			 || (0 == strcasecmp("0", v.c_str())))
		value = false;

	else {
		std::cerr<<"WARN: Resource "<<name<<" value '"<<v<<"' can not be boolean, skipped."<<std::endl;
		return false;
	}
	return true;
}
