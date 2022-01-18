STD = c++17
CONFIG ?= release
CXXFLAGS += -std=$(STD)
LDFLAGS += -lX11 -pthread
CPPCHECKFLAGS = --enable=all --std=$(STD)

ifeq ($(CONFIG), release)
CFLAGS += -Os -g -ggdb3 -pipe -fomit-frame-pointer
CXXFLAGS += -flto -Os -g -ggdb3 -pipe -fno-rtti -fno-stack-protector -ffunction-sections -fdata-sections -fno-unroll-loops -fmerge-all-constants -fno-ident
LDFLAGS += -flto -Wl,-Os -Wl,--gc-sections
else
CXXFLAGS += -Og -g -ggdb3
LDFLAGS += -Og -g -ggdb3
endif

NAME=xanilogo
BIN=$(NAME)
MAN=$(NAME).man
XML=$(NAME).xml

XSS-CONF=$(NAME).conf
XFCE-DESKTOP=$(NAME).desktop

SOURCES=$(wildcard *.cpp)

all: $(BIN)

$(BIN): $(SOURCES)
	g++ $(CPPFLAGS) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

check:
	cppcheck $(CPPCHECKFLAGS) $(SOURCES)

clean:
	rm -f $(BIN)

install: app-install xss-install xfce4-install

app-install: $(BIN)
	install -d '$(DESTDIR)/usr/share/man/man6x'
	gzip <'$(MAN)' >'$(DESTDIR)/usr/share/man/man6x/$(NAME).6x.gz'
	install -D '$(BIN)' '$(DESTDIR)/usr/bin/$(BIN)'
	install -Dm644 '$(XML)' '$(DESTDIR)/usr/share/xscreensaver/config/$(XML)'

xss-install:
	install -Dm644 '$(XSS-CONF)' '$(DESTDIR)/usr/share/xscreensaver/hacks.conf.d/$(XSS-CONF)'
	install -d '$(DESTDIR)/usr/libexec/xscreensaver'
	ln -sf '../../bin/$(BIN)' '$(DESTDIR)/usr/libexec/xscreensaver/$(BIN)'

xfce4-install:
	install -d '$(DESTDIR)/usr/libexec/xfce4-screensaver'
	ln -sf '../../bin/$(BIN)' '$(DESTDIR)/usr/libexec/xfce4-screensaver/$(BIN)'
	install -Dm644 '$(XFCE-DESKTOP)' '$(DESTDIR)/usr/share/applications/screensavers/$(XFCE-DESKTOP)'

xss-update:
	/usr/sbin/update-xscreensaver-hacks

.PHONY: clean install check xss-update
