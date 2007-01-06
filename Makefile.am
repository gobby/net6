ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libnet6.la

#  current  : revision :      age
#     ^     :     ^    :       ^
#     |     :   minor  :       |
# interface :  changes : compatibility
libnet6_la_LDFLAGS = -version-info 0:0:0 -release 1.2

libnet6_la_LIBADD = $(libnet6_LIBS)

if WIN32
libnet6_la_LIBADD += -lws2_32 -lintl
libnet6_la_LDFLAGS += -no-undefined
endif

INCLUDES = $(libnet6_CFLAGS) -Iinc

pkginclude_HEADERS = \
	inc/enum_ops.hpp \
	inc/non_copyable.hpp \
	inc/gettext_package.hpp \
	inc/common.hpp \
	inc/default_accumulator.hpp \
	inc/error.hpp \
	inc/main.hpp \
	inc/serialise.hpp \
	inc/address.hpp \
	inc/socket.hpp \
	inc/select.hpp \
	inc/packet.hpp \
	inc/connection.hpp \
	inc/user.hpp \
	inc/object.hpp \
	inc/local.hpp \
	inc/client.hpp \
	inc/server.hpp \
	inc/host.hpp

libnet6_la_SOURCES = \
	src/non_copyable.cpp \
	src/gettext_package.cpp \
	src/common.cpp \
	src/default_accumulator.cpp \
	src/error.cpp \
	src/main.cpp \
	src/serialise.cpp \
	src/address.cpp \
	src/socket.cpp \
	src/select.cpp \
	src/packet.cpp \
	src/connection.cpp \
	src/user.cpp \
	src/object.cpp \
	src/local.cpp \
	src/client.cpp \
	src/server.cpp \
	src/host.cpp

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = net6-1.2.pc

# gettext
SUBDIRS = po .
EXTRA_DIST = config.rpath mkinstalldirs
libnet6_la_CPPFLAGS = -DLOCALEDIR=\""$(datadir)/locale"\"

# Fix some permissions if they are not set properly when building the tarball
dist-hook:
	chmod u+x $(distdir)/ltmain.sh $(distdir)/missing \
	          $(distdir)/install-sh $(distdir)/config.guess \
	          $(distdir)/config.sub $(distdir)/config.rpath \
	          $(distdir)/depcomp $(distdir)/mkinstalldirs

