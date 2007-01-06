lib_LTLIBRARIES = libnet6.la

INCLUDES = $(libnet6_CFLAGS) -Iinc/

pkginclude_HEADERS = \
	inc/address.hpp \
	inc/client.hpp \
	inc/connection.hpp \
	inc/error.hpp \
	inc/export.hpp \
	inc/main.hpp \
	inc/packet.hpp \
	inc/peer.hpp \
	inc/select.hpp \
	inc/server.hpp \
	inc/socket.hpp

libnet6_la_SOURCES = \
	src/address.cpp \
	src/client.cpp \
	src/connection.cpp \
	src/error.cpp \
	src/main.cpp \
	src/packet.cpp \
	src/peer.cpp \
	src/select.cpp \
	src/server.cpp \
	src/socket.cpp

libnet6_la_LIBADD = $(libnet6_LIBS)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = net6-1.0.pc

