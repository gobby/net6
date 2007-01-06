/* net6 - Library providing IPv4/IPv6 network access
 * Copyright (C) 2005 Armin Burgmeier / 0x539 dev group
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifdef WIN32
#include <winsock2.h>
#else
#include <errno.h>
#endif

#include "error.hpp"

namespace
{
#ifdef WIN32
	const char* wsa_strerror(int code)
	{
		switch(code)
		{
		case WSAEINTR:  return "Interrupted function call";
		case WSAEACCES: return "Access denied";
		case WSAEFAULT: return "Bad address";
		case WSAEINVAL: return "Invalid argument";
		case WSAEMFILE: return "Too many open files";
		case WSAEWOULDBLOCK: return "Resource temporarily unavailable";
		case WSAEINPROGRESS: return "Operation now in progress";
		case WSAEALREADY: return "Operation already in progress";
		case WSAENOTSOCK: return "Socket operation on nonsocket";
		case WSAEDESTADDRREQ: return "Destination address required";
		case WSAEMSGSIZE: return "Message too long";
		case WSAEPROTOTYPE: return "Protocol wrong type for socket";
		case WSAENOPROTOOPT: return "Bad protocol option";
		case WSAEPROTONOSUPPORT: return "Protocol not supported";
		case WSAESOCKTNOSUPPORT: return "Socket type not supported";
		case WSAEOPNOTSUPP: return "Operation not supported";
		case WSAEPFNOSUPPORT: return "Protocol family not supported";
		case WSAEAFNOSUPPORT: return "Address family not supported";
		case WSAEADDRINUSE: return "Address already in use";
		case WSAEADDRNOTAVAIL: return "Cannot assign requested address";
		case WSAENETDOWN: return "Network is down";
		case WSAENETUNREACH: return "Network is unreachable";
		case WSAENETRESET: return "Network dropped connection on reset";
		case WSAECONNABORTED: return "Software caused connection abort";
		case WSAECONNRESET: return "Connection reset by peer";
		case WSAENOBUFS: return "No buffer space available";
		case WSAEISCONN: return "Socket is already connected";
		case WSAENOTCONN: return "Socket is not connected";
		case WSAESHUTDOWN: return "Cannot send after socket shutdown";
		case WSAETIMEDOUT: return "Connection timed out";
		case WSAECONNREFUSED: return "Connection refused";
		case WSAEHOSTDOWN: return "Host is down";
		case WSAEHOSTUNREACH: return "No route to host";
		case WSAEPROCLIM: return "Too many processes";
		case WSASYSNOTREADY: return "Network subsystem is unavailable";
		case WSAVERNOTSUPPORTED:
			return "Winsock.dll version out of range";
		case WSANOTINITIALISED:
			return "Successful WSAStartup not yet performed";
		case WSAEDISCON: return "Graceful shutdown in progress";
		case WSATYPE_NOT_FOUND: return "Class type not found";
		case WSAHOST_NOT_FOUND: return "Host not found";
		case WSATRY_AGAIN: return "Nonauthoritative host not found";
		case WSANO_RECOVERY: return "This is a nonrecoverable error";
		case WSANO_DATA: return "No data record of requested type";
		case WSA_INVALID_HANDLE:
			 return "Specified event object handle is invalid";
		case WSA_INVALID_PARAMETER:
			 return "One or more parameters are invalid";
		default:
			 return "Unknown error occured";
		}
	}
#endif

	const char* error_string(int code)
	{
#ifdef WIN32
		return wsa_strerror(code);
#else
		return strerror(code);
#endif
	}
}

net6::error::error(const char* what)
#ifdef WIN32
 : std::runtime_error(what == NULL ? error_string(WSAGetLastError()) : what)
#else
 : std::runtime_error(what == NULL ? error_string(errno) : what)
#endif
{
}

net6::error::error(int code, const char* what)
 : std::runtime_error(what == NULL ? error_string(code) : what)
{	
}

