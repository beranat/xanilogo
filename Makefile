STD = c++11
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
CONF=$(NAME).conf

SOURCES=$(wildcard *.cpp)

all: $(BIN)

$(BIN): $(SOURCES)
	g++ $(CPPFLAGS) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

check:
	cppcheck $(CPPCHECKFLAGS) $(SOURCES)

clean:
	rm -f $(BIN)

install: $(BIN)
	install -d '$(DESTDIR)/usr/share/man/man6x'
	gzip <'$(MAN)' >'$(DESTDIR)/usr/share/man/man6x/$(NAME).6x.gz'
	install -D '$(BIN)' '$(DESTDIR)/usr/libexec/xscreensaver/$(BIN)'
	install -Dm644 '$(XML)' '$(DESTDIR)/usr/share/xscreensaver/config/$(XML)'
	install -Dm644 '$(CONF)' '$(DESTDIR)/usr/share/xscreensaver/hacks.conf.d/$(CONF)'

update:
	/usr/sbin/update-xscreensaver-hacks

.PHONY: clean install check update
