/* net6 - Library providing IPv4/IPv6 network access
 * Copyright (C) 2005, 2006 Armin Burgmeier / 0x539 dev group
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

#include <fcntl.h>

#include "config.hpp"
#include "encrypt.hpp"

namespace
{
	const unsigned int DH_BITS = 1024;

	net6::gnutls_session_t create_session(net6::gnutls_connection_end_t end)
	{
		net6::gnutls_session_t session;
		gnutls_init(&session, end);
		return session;
	}

#ifdef WIN32
	// Required to turn WSA error codes into errno.
	ssize_t net6_win32_send_func(gnutls_transport_ptr_t ptr,
	                             const void* data,
	                             size_t size)
	{
		ssize_t ret = ::send(
			reinterpret_cast<SOCKET>(ptr),
			static_cast<const char*>(data),
			size,
			0
		);

		int error = WSAGetLastError();
		if(error == WSAEWOULDBLOCK) errno = EAGAIN;
		if(error == WSAEINTR) errno = EINTR;

		// Ensures that a second call to WSAGetLastError
		// has the same result
		WSASetLastError(error);
		return ret;
	}

	ssize_t net6_win32_recv_func(gnutls_transport_ptr_t ptr,
	                             void* data,
	                             size_t size)
	{
		ssize_t ret = ::recv(
			reinterpret_cast<SOCKET>(ptr),
			static_cast<char*>(data),
			size,
			0
		);

		int error = WSAGetLastError();
		if(error == WSAEWOULDBLOCK) errno = EAGAIN;
		if(error == WSAEINTR) errno = EINTR;

		// Ensures that a second call to WSAGetLastError
		// has the same result
		WSASetLastError(error);
		return ret;
	}
#else
	// Send data with MSG_NOSIGNAL
	ssize_t net6_unix_send_func(gnutls_transport_ptr_t ptr,
	                            const void* data,
	                            size_t size)
	{
		// TODO: How to properly get the fd from ptr?
		return send(
			static_cast<int>(reinterpret_cast<intptr_t>(ptr)),
			data,
			size,
#ifdef HAVE_MSG_NOSIGNAL
			// Linux
			MSG_NOSIGNAL
#else
			// Plain BSD
			0
#endif
		);
	}
#endif

	typedef net6::tcp_encrypted_socket_base::size_type
		io_size_type;
	typedef net6::tcp_encrypted_socket_base::handshake_state
		io_handshake_state;

	template<
		typename buffer_type,
		ssize_t(*func)(net6::gnutls_session_t, buffer_type, size_t)
	> io_size_type io_impl(const net6::gnutls_session_t session,
	                       buffer_type buf,
	                       io_size_type len,
	                       io_handshake_state state)
	{
		ssize_t ret;
		switch(state)
		{
		case net6::tcp_encrypted_socket_base::DEFAULT:
			throw std::logic_error(
				"net6::encrypt.cpp:io_impl:\n"
				"Handshake not yet performed"
			);

			break;
		case net6::tcp_encrypted_socket_base::HANDSHAKING:
			throw std::logic_error(
				"net6::encrypt.cpp:io_impl:\n"
				"IO tried while handshaking"
			);

			break;
		case net6::tcp_encrypted_socket_base::HANDSHAKED:
			ret = func(session, buf, len);
			if(ret == GNUTLS_E_AGAIN || ret == GNUTLS_E_INTERRUPTED)
				func(session, NULL, 0);

			if(ret < 0)
				throw net6::error(net6::error::GNUTLS, ret);

			break;
		}

		return ret;
	}
}

net6::dh_params::dh_params():
	params(NULL)
{
	gnutls_dh_params_init(&params);
	gnutls_dh_params_generate2(params, DH_BITS);
}

net6::dh_params::dh_params(gnutls_dh_params_t initial_params):
	params(initial_params)
{
}

net6::dh_params::~dh_params()
{
	gnutls_dh_params_deinit(params);
}

net6::gnutls_dh_params_t net6::dh_params::cobj()
{
	return params;
}

const net6::gnutls_dh_params_t net6::dh_params::cobj() const
{
	return params;
}


net6::tcp_encrypted_socket_base::
	tcp_encrypted_socket_base(socket_type cobj,
                                  gnutls_session_t sess):
	tcp_client_socket(cobj), session(sess), state(DEFAULT)
{
	const int kx_prio[] = { GNUTLS_KX_ANON_DH, 0 };

	gnutls_set_default_priority(session);
	gnutls_kx_set_priority(session, kx_prio);

	gnutls_transport_set_ptr(
		session,
		reinterpret_cast<gnutls_transport_ptr_t>(cobj)
	);

#ifdef WIN32
	gnutls_transport_set_pull_function(
		session,
		net6_win32_recv_func
	);

	gnutls_transport_set_push_function(
		session,
		net6_win32_send_func
	);
#else
	gnutls_transport_set_push_function(
		session,
		net6_unix_send_func
	);
#endif

	gnutls_transport_set_lowat(session, 0);
}

net6::tcp_encrypted_socket_base::~tcp_encrypted_socket_base()
{
	gnutls_bye(session, GNUTLS_SHUT_WR);
	gnutls_deinit(session);
}

bool net6::tcp_encrypted_socket_base::handshake()
{
	if(state == HANDSHAKED)
	{
		throw std::logic_error(
			"net6::tcp_encrypted_socket_base::handshake:\n"
			"Handshake has already been performed"
		);
	}

	if(state == DEFAULT)
	{
#ifdef WIN32
		u_long iMode = 1;
		if(ioctlsocket(cobj(), FIONBIO, &iMode) == SOCKET_ERROR)
			throw net6::error(net6::error::SYSTEM);

		// TODO: How to find out whether the socket is in blocking
		// mode?
		was_blocking = false;
#else
		// Make socket nonblocking to allow to call handshake
		// multiple times
		int flags = fcntl(cobj(), F_GETFL);
		if(fcntl(cobj(), F_SETFL, flags | O_NONBLOCK) == -1)
			throw net6::error(net6::error::SYSTEM);

		was_blocking = ((flags & O_NONBLOCK) == 0);
#endif

		state = HANDSHAKING;
	}

	int ret = gnutls_handshake(session);

	if(ret == 0)
	{
		if(was_blocking)
		{
			// Remove nonblocking state for further handling,
			// so the socket behaves like a nonencrypted tcp
			// client socket.
#ifdef WIN32
		u_long iMode = 0;
		if(ioctlsocket(cobj(), FIONBIO, &iMode) == SOCKET_ERROR)
			throw net6::error(net6::error::SYSTEM);
#else
			int flags = fcntl(cobj(), F_GETFL);
			if(fcntl(cobj(), F_SETFL, flags & ~O_NONBLOCK) == -1)
				throw net6::error(net6::error::SYSTEM);
#endif
		}

		state = HANDSHAKED;
		return true;
	}

	if(ret == GNUTLS_E_AGAIN || ret == GNUTLS_E_INTERRUPTED)
		return false;

	throw net6::error(net6::error::GNUTLS, ret);
}

bool net6::tcp_encrypted_socket_base::get_dir() const
{
	return gnutls_record_get_direction(session) == 1;
}

net6::tcp_encrypted_socket_base::size_type
net6::tcp_encrypted_socket_base::get_pending() const
{
	return gnutls_record_check_pending(session);
}

net6::tcp_encrypted_socket_base::size_type
net6::tcp_encrypted_socket_base::send(const void* buf, size_type len) const
{
	return ::io_impl<const void*, gnutls_record_send>(
		session, buf, len, state
	);
}

net6::tcp_encrypted_socket_base::size_type
net6::tcp_encrypted_socket_base::recv(void* buf, size_type len) const
{
	return ::io_impl<void*, gnutls_record_recv>(
		session, buf, len, state
	);
}

net6::tcp_encrypted_socket_client::
	tcp_encrypted_socket_client(tcp_client_socket& sock):
	tcp_encrypted_socket_base(sock.cobj(), create_session(GNUTLS_CLIENT) )
{
	sock.invalidate();

	gnutls_anon_allocate_client_credentials(&anoncred);
	gnutls_credentials_set(session, GNUTLS_CRD_ANON, anoncred);

	gnutls_dh_set_prime_bits(session, DH_BITS);
}

net6::tcp_encrypted_socket_client::~tcp_encrypted_socket_client()
{
	gnutls_anon_free_client_credentials(anoncred);
}

net6::tcp_encrypted_socket_server::
	tcp_encrypted_socket_server(tcp_client_socket& sock):
	tcp_encrypted_socket_base(sock.cobj(), create_session(GNUTLS_SERVER) ),
	own_params(new dh_params)
{
	sock.invalidate();

	gnutls_anon_allocate_server_credentials(&anoncred);
	gnutls_credentials_set(session, GNUTLS_CRD_ANON, anoncred);

	gnutls_anon_set_server_dh_params(anoncred, own_params->cobj());
}

net6::tcp_encrypted_socket_server::
	tcp_encrypted_socket_server(tcp_client_socket& sock,
	                            dh_params& params):
	tcp_encrypted_socket_base(sock.cobj(), create_session(GNUTLS_SERVER) )
{
	sock.invalidate();

	gnutls_anon_allocate_server_credentials(&anoncred);
	gnutls_credentials_set(session, GNUTLS_CRD_ANON, anoncred);

	gnutls_anon_set_server_dh_params(anoncred, params.cobj() );
}

net6::tcp_encrypted_socket_server::~tcp_encrypted_socket_server()
{
	gnutls_anon_free_server_credentials(anoncred);
}
