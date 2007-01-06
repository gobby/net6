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

#include <iostream>
#include <gnutls/gnutls.h>

#include "error.hpp"
#include "common.hpp"

// Login error codes
const net6::login::error net6::login::ERROR_NAME_INVALID = 0x01;
const net6::login::error net6::login::ERROR_NAME_IN_USE  = 0x02;
const net6::login::error net6::login::ERROR_MAX          = 0xff;

std::string net6::login::errstring(error err)
{
	switch(err)
	{
	case net6::login::ERROR_NAME_INVALID:
		return _("Invalid name");
	case net6::login::ERROR_NAME_IN_USE:
		return _("Name is already in use");
	default:
		return _("An unknown login error occured");
	}
}

namespace
{
	/** Translates a message without being in the net6 namespace.
	 */
	const char* _(const char* msgid)
	{
		return net6::_(msgid);
	}

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

	/** Translates an error code reported by getaddrinfo to a net6 error
	 * code.
	 */
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
		// These #ifdef'd values seem not to exist on all systems.
#ifdef EAI_ADDRFAMILY
		case EAI_ADDRFAMILY: // TODO: Do we want HOST_NOT_FOUND here?
			return net6::error::ADDRESS_UNAVAILABLE;
#endif
#if EAI_NODATA != EAI_NONAME
#ifdef EAI_NODATA
		case EAI_NODATA:
			return net6::error::NO_DATA_RECORD;
#endif
#endif
		case EAI_MEMORY:
			return net6::error::NO_MEMORY;
		case EAI_AGAIN:
			return net6::error::TEMPORARY_FAILURE;
#ifdef EAI_SYSTEM
		case EAI_SYSTEM:
			return system_to_net6(errno);
#endif
		default:
			return net6::error::UNKNOWN;
		}
	}

	/** Translates an error code reported by gethostbyname into a net6
	 * error code.
	 */
	net6::error::code ghbn_to_net6(int code)
	{
#ifdef WIN32
		return system_to_net6(code);
#else
		switch(code)
		{
		case HOST_NOT_FOUND:
			return net6::error::HOSTNAME_NOT_FOUND;
		case NO_ADDRESS:
		//case NO_DATA: // same value as NO_ADDRESS
			return net6::error::NO_DATA_RECORD;
		case TRY_AGAIN:
			return net6::error::TEMPORARY_FAILURE;
		default:
			return net6::error::UNKNOWN;
		}
#endif
	}

	net6::error::code tls_to_net6(int code)
	{
		// TODO: Add more, or, better, remove error code and replace
		// by gnutls_strerror().
		switch(code)
		{
		case GNUTLS_E_AGAIN:
			return net6::error::WOULD_BLOCK;
		case GNUTLS_E_DECRYPTION_FAILED:
			return net6::error::DECRYPTION_FAILED;
		case GNUTLS_E_DH_PRIME_UNACCEPTABLE:
			return net6::error::PRIME_UNACCEPTABLE;
		case GNUTLS_E_ENCRYPTION_FAILED:
			return net6::error::ENCRYPTION_FAILED;
		case GNUTLS_E_GOT_APPLICATION_DATA:
			return net6::error::GOT_APPLICATION_DATA;
		case GNUTLS_E_INSUFFICIENT_CREDENTIALS:
			return net6::error::INSUFFICIENT_CREDENTIALS;
		case GNUTLS_E_INTERRUPTED:
			return net6::error::INTERRUPTED;
		case GNUTLS_E_INVALID_REQUEST:
			return net6::error::INVALID_REQUEST;
		case GNUTLS_E_KEY_USAGE_VIOLATION:
			return net6::error::KEY_USAGE_VIOLATION;
		case GNUTLS_E_MAC_VERIFY_FAILED:
			return net6::error::MAC_VERIFY_FAILED;
		case GNUTLS_E_NO_CERTIFICATE_FOUND:
			return net6::error::NO_CERTIFICATE;
		case GNUTLS_E_NO_TEMPORARY_DH_PARAMS:
			return net6::error::NO_TEMPORARY_DH_PARAMS;
		case GNUTLS_E_NO_TEMPORARY_RSA_PARAMS:
			return net6::error::NO_TEMPORARY_RSA_PARAMS;
		case GNUTLS_E_PK_DECRYPTION_FAILED:
			return net6::error::DECRYPTION_FAILED;
		case GNUTLS_E_PK_ENCRYPTION_FAILED:
			return net6::error::ENCRYPTION_FAILED;
		case GNUTLS_E_PULL_ERROR:
			return net6::error::PULL_ERROR;
		case GNUTLS_E_PUSH_ERROR:
			return net6::error::PUSH_ERROR;
		case GNUTLS_E_RANDOM_FAILED:
			return net6::error::RANDOM_FAILED;
		case GNUTLS_E_RECEIVED_ILLEGAL_PARAMETER:
			return net6::error::INVALID_ARGUMENT;
		case GNUTLS_E_REHANDSHAKE:
			return net6::error::REHANDSHAKE;
		case GNUTLS_E_UNEXPECTED_HANDSHAKE_PACKET:
			return net6::error::UNEXPECTED_HANDSHAKE;
		case GNUTLS_E_UNEXPECTED_PACKET:
			return net6::error::UNEXPECTED_PACKET;
		default:
			std::cerr << "GNUTLS errcode: " << code << std::endl;
			return net6::error::UNKNOWN;
		}
	}

	/** Translates a system dependant error value from a given domain
	 * into a net6 error code
	 */
	net6::error::code domain_to_net6(net6::error::domain error_domain,
	                                 int error_code)
	{
		switch(error_domain)
		{
		case net6::error::SYSTEM:
			return system_to_net6(error_code);
		case net6::error::GETADDRINFO:
			return gai_to_net6(error_code);
		case net6::error::GETHOSTBYNAME:
			return ghbn_to_net6(error_code);
		case net6::error::GNUTLS:
			return tls_to_net6(error_code);
		default:
			throw std::logic_error(
				"domain_to_net6:\n"
				"Unknown error domain"
			);
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
			return _("Interrupted function call");
		case net6::error::ACCESS_DENIED:
			return _("Access denied");
		case net6::error::BAD_ADDRESS:
			return _("Bad address");
		case net6::error::INVALID_ARGUMENT:
			return _("Invalid argument");
		case net6::error::TOO_MANY_FILES:
			return _("Too many open files");
		case net6::error::WOULD_BLOCK:
			return _("Resource temporarily unavailable");
		case net6::error::OPERATION_IN_PROGRESS:
			return _("Operation already in progress");
		case net6::error::NOT_SOCKET:
			return _("Socket operation on non-socket");
		case net6::error::DESTINATION_ADDRESS_REQUIRED:
			return _("Destination address required");
		case net6::error::MESSAGE_TOO_LONG:
			return _("Message too long");
		case net6::error::WRONG_PROTOCOL_TYPE:
			return _("Protocol wrong type for socket");
		case net6::error::BAD_PROTOCOL_OPTION:
			return _("Bad protocol option");
		case net6::error::PROTOCOL_NOT_SUPPORTED:
			return _("Protocol not supported");
		case net6::error::SOCKET_NOT_SUPPORTED:
			return _("Socket type not supported");
		case net6::error::OPERATION_NOT_SUPPORTED:
			return _("Operation not supported");
		case net6::error::PROTOCOL_FAMILY_NOT_SUPPORTED:
			return _("Protocol family not supported");
		case net6::error::ADDRESS_FAMILY_NOT_SUPPORTED:
			return _("Address family not supported");
		case net6::error::ADDRESS_IN_USE:
			return _("Address is already in use");
		case net6::error::ADDRESS_UNAVAILABLE:
			return _("Cannot assign requested address");
		case net6::error::NETWORK_DOWN:
			return _("Network is down");
		case net6::error::NETWORK_UNREACHABLE:
			return _("Network is unreachable");
		case net6::error::NETWORK_RESET:
			return _("Network dropped connection on reset");
		case net6::error::CONNECTION_ABORTED:
			return _("Software caused connection abort");
		case net6::error::CONNECTION_RESET:
			return _("Connection reset by peer");
		case net6::error::NO_BUFFER_SPACE:
			return _("No buffer space available");
		case net6::error::SOCKET_IS_CONNECTED:
			return _("Socket is already connected");
		case net6::error::SOCKET_NOT_CONNECTED:
			return _("Socket is not connected");
		case net6::error::SOCKET_SHUTDOWN:
			return _("Cannot send after socket shutdown");
		case net6::error::CONNECTION_TIMEOUT:
			return _("Connection timed out");
		case net6::error::CONNECTION_REFUSED:
			return _("Connection refused");
		case net6::error::HOST_DOWN:
			return _("Host is down");
		case net6::error::HOST_UNREACHABLE:
			return _("No route to host");
		case net6::error::TOO_MANY_PROCESSES:
			return _("Too many processes");
		case net6::error::SYSTEM_NOT_READY:
			return _("Network subsystem is unavailable");
		case net6::error::VERSION_NOT_SUPPORTED:
			return _("Winsock.dll version out of range");
		case net6::error::NOT_INITIALISED:
			return _("Successful WSAStartup not yet performed");
		case net6::error::DISCONNECTED:
			return _("Graceful shutdown in progress");
		case net6::error::TYPE_NOT_FOUND:
			return _("Class type not found");
		case net6::error::HOSTNAME_NOT_FOUND:
			return _("Host not found");
		case net6::error::TEMPORARY_FAILURE:
			return _("Nonauthoritative host not found");
		case net6::error::NO_DATA_RECORD:
			return _("No data record of requested type");
		case net6::error::INVALID_HANDLE:
			return _("Specified event object handle is invalid");
		case net6::error::INVALID_PARAMETER:
			return _("One or more parameters are invalid");
		case net6::error::NO_MEMORY:
			return _("No more memory is available");
		case net6::error::BROKEN_PIPE:
			return _("Broken pipe");
		case net6::error::NO_DEVICE:
			return _("No such device");
		case net6::error::DECRYPTION_FAILED:
			return _("Decryption has failed");
		case net6::error::PRIME_UNACCEPTABLE:
			return _("The Diffie Hellman prime sent by the server "
			         "is not acceptable (not long enough)");
		case net6::error::ENCRYPTION_FAILED:
			return _("Encryption has failed");
		case net6::error::GOT_APPLICATION_DATA:
			return _("TLS Application data were received, while "
			         "expecting handshake data");
		case net6::error::INSUFFICIENT_CREDENTIALS:
			return _("Insufficient credentials for that request");
		case net6::error::INVALID_REQUEST:
			return _("The request is invalid");
		case net6::error::KEY_USAGE_VIOLATION:
			return _("Key usage violation in certificate has "
			         "been detected");
		case net6::error::MAC_VERIFY_FAILED:
			return _("The Message Authentication Code "
			         "verification failed");
		case net6::error::NO_CERTIFICATE:
			return _("The peer did not send any certificate");
		case net6::error::NO_TEMPORARY_DH_PARAMS:
			return _("No temporary DH parameters were found");
		case net6::error::NO_TEMPORARY_RSA_PARAMS:
			return _("No temporary RSA parameters were found");
		case net6::error::PULL_ERROR:
			return _("Error in the pull function");
		case net6::error::PUSH_ERROR:
			return _("Error in the push function");
		case net6::error::RANDOM_FAILED:
			return _("Failed to acquire random data");
		case net6::error::REHANDSHAKE:
			return _("Rehandshake was requested by the peer");
		case net6::error::UNEXPECTED_HANDSHAKE:
			return _("An unexpected TLS handshake packet "
			         "was received");
		case net6::error::UNEXPECTED_PACKET:
			return _("An unexpected TLS packet was received");
		case net6::error::UNKNOWN:
			return _("A nonrecoverable error has occured");
		default:
			throw std::logic_error(
				"net6_strerror:\n"
				"Unknown error code"
			);
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

net6::error::error(domain error_domain):
	std::runtime_error(
		net6_strerror(
			errcode = domain_to_net6(
				error_domain,
				last_error(error_domain)
			)
		)
	)
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

