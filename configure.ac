#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([net6], [1.3.5], [armin@0x539.de])
AM_INIT_AUTOMAKE(1.9 check-news)
AM_MAINTAINER_MODE
AC_CONFIG_SRCDIR([inc/main.hpp])
AC_CONFIG_HEADER([inc/config.hpp])

# Extract host information.
AC_CANONICAL_HOST

# Checks for programs.
AC_LANG([C++])
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL
AC_CACHE_SAVE

# Use C++ compiler for the tests.
AC_LANG([C++])

# Check for headers.
AC_CHECK_HEADERS([sys/select.h])

# Check for MSG_NOSIGNAL
AC_MSG_CHECKING(for MSG_NOSIGNAL)
AC_TRY_COMPILE([#include <sys/socket.h>],
	       [ int f = MSG_NOSIGNAL; ],
	       [ AC_MSG_RESULT(yes)
	         AC_DEFINE(HAVE_MSG_NOSIGNAL, 1,
			   [Define this symbol if you have MSG_NOSIGNAL]) ],
 	       [ AC_MSG_RESULT(no) ])
AC_MSG_CHECKING(for SO_NOSIGPIPE)
AC_TRY_COMPILE([#include <sys/socket.h>],
               [ int f = SO_NOSIGNAL; ],
               [ AC_MSG_RESULT(yes)
                 AC_DEFINE(HAVE_SO_NOSIGNAL, 1,
                           [Define this symbol if you have SO_NOSIGNAL]) ],
               [ AC_MSG_RESULT(no) ])
AC_MSG_CHECKING(for AI_ADDRCONFIG)
AC_TRY_COMPILE([#include <sys/types.h>
                #include <sys/socket.h>
                #include <netdb.h>],
               [ struct addrinfo hint;
                 hint.ai_flags = AI_ADDRCONFIG; ],
               [ AC_MSG_RESULT(yes)
                 AC_DEFINE(HAVE_AI_ADDRCONFIG, 1,
                           [Define this symbol if you have AI_ADDRCONFIG]) ],
               [ AC_MSG_RESULT(no) ])

AC_MSG_CHECKING([whether to enable WIN32 specific flags])
case "$host_os" in
*mingw*)
	win32=true
	addlibs="-lws2_32" 
	AC_MSG_RESULT([yes])
	;;
*)
	win32=false
	addlibs=""
	AC_MSG_RESULT([no])
	;;
esac
AC_SUBST(addlibs)
AM_CONDITIONAL(WIN32, test x$win32 = xtrue)

# Checks for libraries.
PKG_CHECK_MODULES([libnet6], [sigc++-2.0])

AM_PATH_LIBGNUTLS( 1.0.0,, AC_MSG_ERROR([[
***
*** libgnutls was not found. You may want to get it by using the
*** package manager of your distribution or by compiling the source
*** from ftp://ftp.gnutls.org/pub/gnutls/. This dependency is
*** now mandatory.
]]))
addlibs="$addlibs $LIBGNUTLS_LIBS"
addcflags="$LIBGNUTLS_CFLAGS"
AC_SUBST(addcflags)

# gettext / i18n
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.15])

AC_CONFIG_FILES([po/Makefile.in Makefile])
AC_OUTPUT([net6-1.3.pc])

