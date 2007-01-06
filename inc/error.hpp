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

#ifndef _NET6_ERROR_HPP_
#define _NET6_ERROR_HPP_

#include <stdexcept>

#include "export.hpp"

namespace net6
{

/** Common net6 error reporting class.
 */
	
class NET6_EXPORT error : public std::runtime_error
{
public:
	/** Error domain. Used by the error class to translate system
	 * dependant error values to net6 error codes.
	 */
	enum domain {
		SYSTEM, // WSAGetLastError or errno
		GETADDRINFO // getaddrinfo
	};

	/** Taken from Winsock error code documentation and slightly renamed.
	 */
	enum code {
		INTERRUPTED, // Interrupted function call
		ACCESS_DENIED, // Access denied
		BAD_ADDRESS, // Bad address
		INVALID_ARGUMENT, // Invalid argument
		TOO_MANY_FILES, // Too many open files
		WOULD_BLOCK, // Resource temporariliy unavailable
		OPERATION_IN_PROGRESS, // Operation already in progress
//		OPERATION_ALREADY, // Operation already in progress
		NOT_SOCKET, // Socket operation on non-socket
		DESTINATION_ADDRESS_REQUIRED, // Destination address required
		MESSAGE_TOO_LONG, // Message too long
		WRONG_PROTOCOL_TYPE, // Wrong protocol type for socket
		BAD_PROTOCOL_OPTION, // Bad protocol option
		PROTOCOL_NOT_SUPPORTED, // Protocol not supported
		SOCKET_NOT_SUPPORTED, // Socket type not supported
		OPERATION_NOT_SUPPORTED, // Operation not supported
		PROTOCOL_FAMILY_NOT_SUPPORTED, // Protocol family not supported
		ADDRESS_FAMILY_NOT_SUPPORTED, // Address family not supported
		ADDRESS_IN_USE, // Address is already in use
		ADDRESS_UNAVAILABLE, // Cannot assign requested address
		NETWORK_DOWN, // Network is down
		NETWORK_UNREACHABLE, // Network is unreachable
		NETWORK_RESET, // Network dropped connection on reset
		CONNECTION_ABORTED, // Software caused connection abort
		CONNECTION_RESET, // Connection reset by peer
		NO_BUFFER_SPACE, // No buffer space available
		SOCKET_IS_CONNECTED, // Socket is already connected
		SOCKET_NOT_CONNECTED, // Socket is not connected
		SOCKET_SHUTDOWN, // Cannot send after socket shutdown
		CONNECTION_TIMEOUT, // Connection timed out
		CONNECTION_REFUSED, // Connection refused
		HOST_DOWN, // Host is down
		HOST_UNREACHABLE, // No route to host
		TOO_MANY_PROCESSES, // Too many processes
		SYSTEM_NOT_READY, // Network subsystem is unavailable
		VERSION_NOT_SUPPORTED, // Winsock.dll version out of range
		NOT_INITIALISED, // Successful WSAStartup not yet performed
		DISCONNECTED, // Graceful shutdown in progress
		TYPE_NOT_FOUND, // Class type not found
		HOSTNAME_NOT_FOUND, // Host not found
		TEMPORARY_FAILURE, // Nonauthoritative host not found
		NO_DATA_RECORD, // No data record of requested type
		INVALID_HANDLE, // Specified event object handle is invalid
		INVALID_PARAMETER, // One or more parameters are invalid
		NO_MEMORY, // Insufficient memory is available
		BROKEN_PIPE, // The connection was unexpectedly closed
		NO_DEVICE, // Network device not available

		UNKNOWN // This is a nonrecoverable error
	};

	/** Generate error by an error code from a system function call.
	 */
	error(domain error_domain, int error_code);

	/** Gernate error by an error domain and the last occured error.
	 */
	error(domain error_domain);

	/** Generate error by net6 error code.
	 */
	error(code error_code);
};

}

#endif
