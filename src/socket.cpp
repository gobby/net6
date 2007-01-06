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

#include "error.hpp"
#include "socket.hpp"

#ifdef WIN32
#define WIN32_CAST_FIX(a) (static_cast<char*>(a) )
#define WIN32_CCAST_FIX(a) (static_cast<const char*>(a) )
#else
#define WIN32_CAST_FIX(a) (a)
#define WIN32_CCAST_FIX(a) (a)
#endif

namespace
{
	const unsigned int DH_BITS = 1024;

	int address_to_protocol(int af)
	{
		switch(af)
		{
		case AF_INET: return PF_INET;
		case AF_INET6: return PF_INET6;
		default:
			throw net6::error(
				net6::error::ADDRESS_FAMILY_NOT_SUPPORTED
			);
		}
	}

	net6::gnutls_session_t create_session(net6::gnutls_connection_end_t end)
	{
		net6::gnutls_session_t session;
		gnutls_init(&session, end);
		return session;
	}

#ifdef WIN32
	// Required to turn WSA error codes into errno.
	template<
		typename buffer_type,
		typename cast_type,
		ssize_t(*io_func)(SOCKET, cast_type, size_t, int)
	>
	ssize_t net6_win32_io_func(gnutls_transport_ptr_t ptr,
	                           buffer_type data,
	                           size_t size)
	{
		ssize_t ret = io_func(
			reinterpret_cast<SOCKET>(ptr),
			static_cast<cast_type>(data),
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
			MSG_NOSIGNAL
		);
	}
#endif

#ifndef WIN32
	const int INVALID_SOCKET = -1;
#endif
}

net6::socket::socket(int domain, int type, int protocol):
	sock(::socket(domain, type, protocol) )
{
	if(sock == INVALID_SOCKET)
		throw error(error::SYSTEM);
}

net6::socket::socket(socket_type c_object):
	sock(c_object)
{
}

net6::socket::~socket()
{
	if(sock != INVALID_SOCKET)
	{
#ifdef WIN32
		closesocket(cobj() );
#else
		close(cobj() );
#endif
	}
}

void net6::socket::invalidate()
{
	sock = INVALID_SOCKET;
}

net6::tcp_socket::tcp_socket(const address& addr):
	socket(address_to_protocol(addr.get_family()), SOCK_STREAM, IPPROTO_TCP)
{
}

net6::tcp_socket::tcp_socket(socket_type c_object):
	socket(c_object)
{
}

net6::tcp_client_socket::tcp_client_socket(const address& addr):
	tcp_socket(addr)
{
	if(::connect(cobj(), addr.cobj(), addr.get_size()) == -1)
		throw error(net6::error::SYSTEM);
}

net6::tcp_client_socket::tcp_client_socket(socket_type c_object):
	tcp_socket(c_object)
{
}

net6::tcp_client_socket::~tcp_client_socket()
{
}

net6::socket::size_type net6::tcp_client_socket::send(const void* buf,
                                                      size_type len) const
{
	ssize_t result = ::send(cobj(), WIN32_CCAST_FIX(buf), len, 0);
	if(result < 0)
		throw error(net6::error::SYSTEM);

	return result;
}

net6::socket::size_type net6::tcp_client_socket::recv(void* buf,
                                                      size_type len) const
{
	ssize_t result = ::recv(cobj(), WIN32_CAST_FIX(buf), len, 0);
	if(result < 0)
		throw error(net6::error::SYSTEM);

	return result;
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
		net6_win32_io_func<void*, char*, ::recv>
	);

	gnutls_transport_set_push_function(
		session.
		net6_win32_io_func<const void*, const char*, ::send>
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
	return io_impl<const void*, gnutls_record_send>(buf, len);
}

net6::tcp_encrypted_socket_base::size_type
net6::tcp_encrypted_socket_base::recv(void* buf, size_type len) const
{
	return io_impl<void*, gnutls_record_recv>(buf, len);
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
	tcp_encrypted_socket_base(sock.cobj(), create_session(GNUTLS_SERVER) )
{
	sock.invalidate();

	gnutls_anon_allocate_server_credentials(&anoncred);
	gnutls_credentials_set(session, GNUTLS_CRD_ANON, anoncred);

	// TODO: Generate diffie-hellman parameters somewhere else
	gnutls_dh_params_init(&dh_params);
	gnutls_dh_params_generate2(dh_params, DH_BITS);
	gnutls_anon_set_server_dh_params(anoncred, dh_params);
}

net6::tcp_encrypted_socket_server::~tcp_encrypted_socket_server()
{
	gnutls_dh_params_deinit(dh_params);
	gnutls_anon_free_server_credentials(anoncred);
}

net6::tcp_server_socket::tcp_server_socket(const address& bind_addr):
	tcp_socket(bind_addr)
{
	if(bind(cobj(), bind_addr.cobj(), bind_addr.get_size()) == -1)
		throw error(net6::error::SYSTEM);
	if(listen(cobj(), 0) == -1)
		throw error(net6::error::SYSTEM);
}

net6::tcp_server_socket::tcp_server_socket(socket_type c_object):
	tcp_socket(c_object)
{
}

std::auto_ptr<net6::tcp_client_socket> net6::tcp_server_socket::accept() const
{
	socket_type new_sock = ::accept(cobj(), NULL, NULL);
	if(new_sock == INVALID_SOCKET)
		throw error(net6::error::SYSTEM);

	return std::auto_ptr<tcp_client_socket>(
		new tcp_client_socket(new_sock)
	);
}

std::auto_ptr<net6::tcp_client_socket>
net6::tcp_server_socket::accept(address& from) const
{
	socklen_t sock_size = from.get_size();
	socket_type new_sock = ::accept(cobj(), from.cobj(), &sock_size);
	if(new_sock == INVALID_SOCKET)
		throw error(net6::error::SYSTEM);

	return std::auto_ptr<tcp_client_socket>(
		new tcp_client_socket(new_sock)
	);
}

net6::udp_socket::udp_socket(const address& bind_addr):
	socket(
		address_to_protocol(bind_addr.get_family()),
		SOCK_DGRAM,
		IPPROTO_UDP
	)
{
	if(::bind(cobj(), bind_addr.cobj(), bind_addr.get_size()) == -1)
		throw error(net6::error::SYSTEM);
}

void net6::udp_socket::set_target(const address& addr)
{
	if(connect(cobj(), addr.cobj(), addr.get_size()) == -1)
		throw error(net6::error::SYSTEM);
}

void net6::udp_socket::reset_target()
{
	if(connect(cobj(), NULL, 0) == -1)
		throw error(net6::error::SYSTEM);
}

net6::socket::size_type
net6::udp_socket::send(const void* buf, size_type len) const
{
	ssize_t result = ::send(cobj(), WIN32_CCAST_FIX(buf), len, 0);
	if(result == -1)
		throw error(net6::error::SYSTEM);

	return result;
}

net6::socket::size_type
net6::udp_socket::send(const void* buf, size_type len, const address& to) const
{
	ssize_t result = ::sendto(cobj(), WIN32_CCAST_FIX(buf), len, 0,
	                          to.cobj(), to.get_size());
	if(result == -1)
		throw error(net6::error::SYSTEM);

	return result;
}

net6::socket::size_type
net6::udp_socket::recv(void* buf, size_type len) const
{
	ssize_t result = ::recv(cobj(), WIN32_CAST_FIX(buf), len, 0);
	if(result == -1)
		throw error(net6::error::SYSTEM);

	return result;
}

net6::socket::size_type
net6::udp_socket::recv(void* buf, size_type len, address& from) const
{
	socklen_t sock_size = from.get_size();
	ssize_t result = ::recvfrom(cobj(), WIN32_CAST_FIX(buf), len, 0,
	                            from.cobj(), &sock_size);
	if(result == -1)
		throw error(net6::error::SYSTEM);

	return result;
}

