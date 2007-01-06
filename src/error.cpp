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
#include <ws2tcpip.h>
#else
#include <errno.h>
#include <netdb.h>
#endif

#include "error.hpp"

namespace
{
	/** Translates a system dependant error value to net6 error codes.
	 */
	net6::error::code system_to_net6(int error_code)
	{
		switch(error_code)
		{
#ifdef WIN32
		case WSAEINTR:
			return net6::error::INTERRUPTED;
		case WSAEACCES:
			return net6::error::ACCESS_DENIED;
		case WSAEFAULT:
			return net6::error::BAD_ADDRESS;
		case WSAEINVAL:
			return net6::error::INVALID_ARGUMENT;
		case WSAEMFILE:
			return net6::error::TOO_MANY_FILES;
		case WSAEWOULDBLOCK:
			return net6::error::WOULD_BLOCK;
		case WSAEALREADY:
			return net6::error::OPERATION_IN_PROGRESS;
		case WSAENOTSOCK:
			return net6::error::NOT_SOCKET;
		case WSAEDESTADDRREQ:
			return net6::error::DESTINATION_ADDRESS_REQUIRED;
		case WSAEMSGSIZE:
			return net6::error::MESSAGE_TOO_LONG;
		case WSAEPROTOTYPE:
			return net6::error::WRONG_PROTOCOL_TYPE;
		case WSAENOPROTOOPT:
			return net6::error::BAD_PROTOCOL_OPTION;
		case WSAEPROTONOSUPPORT: 
			return net6::error::PROTOCOL_NOT_SUPPORTED;
		case WSAESOCKTNOSUPPORT:
			return net6::error::SOCKET_NOT_SUPPORTED;
		case WSAEOPNOTSUPP:
			return net6::error::OPERATION_NOT_SUPPORTED;
		case WSAEPFNOSUPPORT:
			return net6::error::PROTOCOL_FAMILY_NOT_SUPPORTED;
		case WSAEAFNOSUPPORT:
			return net6::error::ADDRESS_FAMILY_NOT_SUPPORTED;
		case WSAEADDRINUSE:
			return net6::error::ADDRESS_IN_USE;
		case WSAEADDRNOTAVAIL:
			return net6::error::ADDRESS_UNAVAILABLE;
		case WSAENETDOWN:
			return net6::error::NETWORK_DOWN;
		case WSAENETUNREACH:
			return net6::error::NETWORK_UNREACHABLE;
		case WSAENETRESET:
			return net6::error::NETWORK_RESET;
		case WSAECONNABORTED:
			return net6::error::CONNECTION_ABORTED;
		case WSAECONNRESET:
			return net6::error::CONNECTION_RESET;
		case WSAENOBUFS:
			return net6::error::NO_BUFFER_SPACE;
		case WSAEISCONN:
			return net6::error::SOCKET_IS_CONNECTED;
		case WSAENOTCONN:
			return net6::error::SOCKET_NOT_CONNECTED;
		case WSAESHUTDOWN:
			return net6::error::SOCKET_SHUTDOWN;
		case WSAETIMEDOUT:
			return net6::error::CONNECTION_TIMEOUT;
		case WSAECONNREFUSED:
			return net6::error::CONNECTION_REFUSED;
		case WSAEHOSTDOWN:
			return net6::error::HOST_DOWN;
		case WSAEHOSTUNREACH:
			return net6::error::HOST_UNREACHABLE;
		case WSAEPROCLIM:
			return net6::error::TOO_MANY_PROCESSES;
		case WSASYSNOTREADY:
			return net6::error::SYSTEM_NOT_READY;
		case WSAVERNOTSUPPORTED:
			return net6::error::VERSION_NOT_SUPPORTED;
		case WSANOTINITIALISED:
			return net6::error::NOT_INITIALISED;
		case WSAEDISCON:
			return net6::error::DISCONNECTED;
		case WSATYPE_NOT_FOUND:
			return net6::error::TYPE_NOT_FOUND;
		case WSAHOST_NOT_FOUND:
			return net6::error::HOSTNAME_NOT_FOUND;
		case WSATRY_AGAIN:
			return net6::error::TEMPORARY_FAILURE;
		case WSANO_DATA:
			return net6::error::NO_DATA_RECORD;
		case WSA_INVALID_HANDLE:
			return net6::error::INVALID_HANDLE;
		case WSA_INVALID_PARAMETER:
			return net6::error::INVALID_PARAMETER;
#else
		case EACCES:
			return net6::error::ACCESS_DENIED;
		case EADDRINUSE:
			return net6::error::ADDRESS_IN_USE;
		case EADDRNOTAVAIL:
			return net6::error::ADDRESS_UNAVAILABLE;
		case EAFNOSUPPORT:
			return net6::error::ADDRESS_FAMILY_NOT_SUPPORTED;
		case EAGAIN:
			return net6::error::WOULD_BLOCK;
		case EALREADY:
			return net6::error::OPERATION_IN_PROGRESS;
		case EBADF:
			return net6::error::NOT_SOCKET;
		// TODO: What is bad message..?
		// deactivated 2005-04-15 by phil: not BSD-compatible
//		case EBADMSG:
//			return net6::error::MESSAGE_TOO_LONG;
		case ECONNABORTED:
			return net6::error::CONNECTION_ABORTED;
		case ECONNREFUSED:
			return net6::error::CONNECTION_REFUSED;
		case ECONNRESET:
			return net6::error::CONNECTION_RESET;
		case EDESTADDRREQ:
			return net6::error::DESTINATION_ADDRESS_REQUIRED;
		case EFAULT:
			return net6::error::BAD_ADDRESS;
		case EHOSTUNREACH:
			return net6::error::HOST_UNREACHABLE;
		case EINPROGRESS:
			return net6::error::OPERATION_IN_PROGRESS;
		case EINTR:
			return net6::error::INTERRUPTED;
		case EINVAL:
			return net6::error::INVALID_ARGUMENT;
		case EISCONN:
			return net6::error::SOCKET_IS_CONNECTED;
		case EMFILE:
			return net6::error::TOO_MANY_FILES;
		case EMSGSIZE:
			return net6::error::MESSAGE_TOO_LONG;
		case ENETDOWN:
			return net6::error::NETWORK_DOWN;
		case ENETRESET:
			return net6::error::NETWORK_RESET;
		case ENETUNREACH:
			return net6::error::NETWORK_UNREACHABLE;
		case ENFILE:
			return net6::error::TOO_MANY_FILES;
		case ENOBUFS:
			return net6::error::NO_BUFFER_SPACE;
		// deactivated 2005-04-15 by phil: not BSD-compatible
//		case ENODATA:
//			return net6::error::NO_DATA_RECORD;
		case ENODEV:
			return net6::error::NO_DEVICE;
		case ENOMEM:
			return net6::error::NO_MEMORY;
		case ENOPROTOOPT:
			return net6::error::BAD_PROTOCOL_OPTION;
		case ENOTCONN:
			return net6::error::SOCKET_NOT_CONNECTED;
		case ENOTSOCK:
			return net6::error::NOT_SOCKET;
		case EOPNOTSUPP:
			return net6::error::OPERATION_NOT_SUPPORTED;
		case EPERM:
			return net6::error::ACCESS_DENIED;
		case EPIPE:
			return net6::error::BROKEN_PIPE;
		case EPROTONOSUPPORT:
			return net6::error::PROTOCOL_NOT_SUPPORTED;
		case EPROTOTYPE:
			return net6::error::WRONG_PROTOCOL_TYPE;
		case ETIMEDOUT:
			return net6::error::CONNECTION_TIMEOUT;
//		case EWOULDBLOCK: // Same as EAGAIN
//			return net6::error::WOULD_BLOCK;
#endif
		default:
			return net6::error::UNKNOWN;
		}
	}

	net6::error::code gai_to_net6(int code)
	{
		using net6::error;
		switch(code)
		{
		case EAI_FAMILY:
			return net6::error::ADDRESS_FAMILY_NOT_SUPPORTED;
		case EAI_SOCKTYPE:
			return net6::error::SOCKET_NOT_SUPPORTED;
		case EAI_BADFLAGS:
			return net6::error::INVALID_ARGUMENT;
		case EAI_NONAME:
			return net6::error::HOSTNAME_NOT_FOUND;
		case EAI_SERVICE:
			return net6::error::TYPE_NOT_FOUND;
#ifndef WIN32
		case EAI_ADDRFAMILY: // TODO: Do we want HOST_NOT_FOUND here?
			return net6::error::ADDRESS_UNAVAILABLE;
#endif
		case EAI_NODATA:
			return net6::error::NO_DATA_RECORD;
		case EAI_MEMORY:
			return net6::error::NO_MEMORY;
		case EAI_AGAIN:
			return net6::error::TEMPORARY_FAILURE;
#ifndef WIN32
		case EAI_SYSTEM:
			return system_to_net6(errno);
#endif
		default:
			return net6::error::UNKNOWN;
		}
	}

	/** Translates a system dependant error value from a given domain
	 * into a net6 error code
	 */
	net6::error::code domain_to_net6(net6::error::domain error_domain,
	                                 int error_code)
	{
		using net6::error;
		switch(error_domain)
		{
		case net6::error::SYSTEM:
			return system_to_net6(error_code);
		case net6::error::GETADDRINFO:
			return gai_to_net6(error_code);
		}
	}

	/** Translates an error code into a human-readable error message
	 */
	const char* net6_strerror(net6::error::code error_code)
	{
		using net6::error;
		switch(error_code)
		{
		case net6::error::INTERRUPTED:
			return "Interrupted function call";
		case net6::error::ACCESS_DENIED:
			return "Access denied";
		case net6::error::BAD_ADDRESS:
			return "Bad address";
		case net6::error::INVALID_ARGUMENT:
			return "Invalid argument";
		case net6::error::TOO_MANY_FILES:
			return "Too many open files";
		case net6::error::WOULD_BLOCK:
			return "Resource temporarily unavailable";
		case net6::error::OPERATION_IN_PROGRESS:
			return "Operation already in progress";
		case net6::error::NOT_SOCKET:
			return "Socket operation on non-socket";
		case net6::error::DESTINATION_ADDRESS_REQUIRED:
			return "Destination address required";
		case net6::error::MESSAGE_TOO_LONG:
			return "Message too long";
		case net6::error::WRONG_PROTOCOL_TYPE:
			return "Protocol wrong type for socket";
		case net6::error::BAD_PROTOCOL_OPTION:
			return "Bad protocol option";
		case net6::error::PROTOCOL_NOT_SUPPORTED:
			return "Protocol not supported";
		case net6::error::SOCKET_NOT_SUPPORTED:
			return "Socket type not supported";
		case net6::error::OPERATION_NOT_SUPPORTED:
			return "Operation not supported";
		case net6::error::PROTOCOL_FAMILY_NOT_SUPPORTED:
			return "Protocol family not supported";
		case net6::error::ADDRESS_FAMILY_NOT_SUPPORTED:
			return "Address family not supported";
		case net6::error::ADDRESS_IN_USE:
			return "Address is already in use";
		case net6::error::ADDRESS_UNAVAILABLE:
			return "Cannot assign requested address";
		case net6::error::NETWORK_DOWN:
			return "Network is down";
		case net6::error::NETWORK_UNREACHABLE:
			return "Network is unreachable";
		case net6::error::NETWORK_RESET:
			return "Network dropped connection on reset";
		case net6::error::CONNECTION_ABORTED:
			return "Software caused connection abort";
		case net6::error::CONNECTION_RESET:
			return "Connection reset by peer";
		case net6::error::NO_BUFFER_SPACE:
			return "No buffer space available";
		case net6::error::SOCKET_IS_CONNECTED:
			return "Socket is already connected";
		case net6::error::SOCKET_NOT_CONNECTED:
			return "Socket is not connected";
		case net6::error::SOCKET_SHUTDOWN:
			return "Cannot send after socket shutdown";
		case net6::error::CONNECTION_TIMEOUT:
			return "Connection timed out";
		case net6::error::CONNECTION_REFUSED:
			return "Connection refused";
		case net6::error::HOST_DOWN:
			return "Host is down";
		case net6::error::HOST_UNREACHABLE:
			return "No route to host";
		case net6::error::TOO_MANY_PROCESSES:
			return "Too many processes";
		case net6::error::SYSTEM_NOT_READY:
			return "Network subsystem is unavailable";
		case net6::error::VERSION_NOT_SUPPORTED:
			return "Winsock.dll version out of range";
		case net6::error::NOT_INITIALISED:
			return "Successful WSAStartup not yet performed";
		case net6::error::DISCONNECTED:
			return "Graceful shutdown in progress";
		case net6::error::TYPE_NOT_FOUND:
			return "Class type not found";
		case net6::error::HOSTNAME_NOT_FOUND:
			return "Host not found";
		case net6::error::TEMPORARY_FAILURE:
			return "Nonauthoritative host not found";
		case net6::error::NO_DATA_RECORD:
			return "No data record of requested type";
		case net6::error::INVALID_HANDLE:
			return "Specified event object handle is invalid";
		case net6::error::INVALID_PARAMETER:
			return "One or more parameters are invalid";
		case net6::error::NO_MEMORY:
			return "No more memory is available";
		case net6::error::BROKEN_PIPE:
			return "Broken pipe";
		case net6::error::NO_DEVICE:
			return "No such device";
		case net6::error::UNKNOWN:
			return "A nonrecoverable error has occured";
		}
	}

	int last_error(net6::error::domain error_domain)
	{
		switch(error_domain)
		{
		case net6::error::SYSTEM:
#ifdef WIN32
			return WSAGetLastError();
#else
			return errno;
#endif
		default:
			return -1;
		}
	}
}

net6::error::error(domain error_domain, int error_code)
 : std::runtime_error(net6_strerror(domain_to_net6(error_domain, error_code)) ),
   errcode(domain_to_net6(error_domain, error_code) )
{
}

net6::error::error(domain error_domain)
 : std::runtime_error(
	net6_strerror(domain_to_net6(error_domain, last_error(error_domain)) )
   ), 
   errcode(domain_to_net6(error_domain, last_error(error_domain)))
{
}
		

net6::error::error(code error_code)
 : std::runtime_error(net6_strerror(error_code) ), errcode(error_code)
{
}

net6::error::code net6::error::get_code() const
{
	return errcode;
}

