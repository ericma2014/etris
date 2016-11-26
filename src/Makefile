CFLAGS += -std=c99 -ggdb -O0
LDFLAGS +=
CPPFLAGS +=

VER_MAJOR = 0
VER_MINOR = 0
VER_PATCH = 1
VER=$(VER_MAJOR).$(VER_MINOR).$(VER_PATCH)

PREFIX ?= /usr/local
LIBDIR = $(PREFIX)/lib
INCLUDEDIR = $(PREFIX)/include
DESTDIR ?=
INSTALL = /bin/install -c
MKDIR_P = /bin/mkdir -p
CP = /bin/cp -fd
LN = /bin/ln -fs

# build shared lib under OS X or Linux
OS = $(shell uname -s)
ifeq ($(OS),Darwin)
	SHAREDLIB_LINK_OPTIONS=-dynamiclib -Wl,-install_name -Wl,
else
	SHAREDLIB_LINK_OPTIONS=-shared -Wl,-soname,
endif

# targets to build with 'make all'
TARGETS = etris-sdl libetris.a libetris.so

all: $(TARGETS)

etris-sdl: etris-sdl.o libetris.a
	$(CC) -o $@ $^ $(shell sdl-config --libs) -lSDL_ttf

etris-sdl.o: etris-sdl.c etris.h
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(shell sdl-config --cflags) -o $@ etris-sdl.c

libetris.a: etris.o
	$(AR) -cvq $@ $^

libetris.so: etris.o
	$(CC) $(SHAREDLIB_LINK_OPTIONS)$@.$(VER_MAJOR) -o $@.$(VER) $^
	$(LN) $@.$(VER) $@.$(VER_MAJOR)
	$(LN) $@.$(VER_MAJOR) $@

etris.o: etris.c etris.h Makefile
	$(CC) -c -fPIC $(CFLAGS) $(CPPFLAGS) -o $@ etris.c

install: all installdirs
	$(INSTALL) -m644 *.h $(DESTDIR)$(INCLUDEDIR)
	$(CP) *.so* *.a $(DESTDIR)$(LIBDIR)

installdirs:
	$(MKDIR_P) $(DESTDIR)$(LIBDIR) $(DESTDIR)$(INCLUDEDIR)

clean:
	rm -f *.o *~ *.so* $(TARGETS)
