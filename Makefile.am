lib_LTLIBRARIES = libnet6.la

#  current  : revision :      age
#     ^     :     ^    :       ^
#     |     :   minor  :       |
# interface :  changes : compatibility
libnet6_la_LDFLAGS = -version-info 0:0:0 -release 1 

libnet6_la_LIBADD = $(libnet6_LIBS)

if WIN32
libnet6_la_LIBADD += -lws2_32
libnet6_la_LDFLAGS += -no-undefined
endif

INCLUDES = $(libnet6_CFLAGS) -Iinc

pkginclude_HEADERS = \
	inc/non_copyable.hpp \
	inc/address.hpp \
	inc/client.hpp \
	inc/connection.hpp \
	inc/error.hpp \
	inc/main.hpp \
	inc/packet.hpp \
	inc/peer.hpp \
	inc/select.hpp \
	inc/server.hpp \
	inc/host.hpp \
	inc/socket.hpp

libnet6_la_SOURCES = \
	src/non_copyable.cpp \
	src/address.cpp \
	src/client.cpp \
	src/connection.cpp \
	src/error.cpp \
	src/main.cpp \
	src/packet.cpp \
	src/peer.cpp \
	src/select.cpp \
	src/server.cpp \
	src/host.cpp \
	src/socket.cpp

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = net6-1.0.pc

