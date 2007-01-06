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

namespace
{
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
}

#ifdef WIN32
#define WIN32_CAST_FIX(a) (static_cast<char*>(a) )
#define WIN32_CCAST_FIX(a) (static_cast<const char*>(a) )
#else
#define WIN32_CAST_FIX(a) (a)
#define WIN32_CCAST_FIX(a) (a)
#endif

#ifndef WIN32
namespace { const int INVALID_SOCKET = -1; }
#endif

net6::socket::socket(int domain, int type, int protocol)
 : data(new socket_data)
{
	data->sock = ::socket(domain, type, protocol);
	data->refcount = 1;
}

net6::socket::socket(socket_type c_object)
 : data(new socket_data)
{
	data->sock = c_object;
	data->refcount = 1;
}

net6::socket::socket(const socket& other)
 : data(other.data)
{
	data->refcount ++;
}

net6::socket::~socket()
{
	data->refcount --;

	if(!data->refcount)
	{
#ifdef WIN32
		closesocket(cobj() );
#else
		close(cobj() );
#endif
		delete data;
	}
}

net6::socket& net6::socket::operator=(const socket& other)
{
	if(this == &other)
		return *this;

	data->refcount --;
	if(!data->refcount)
	{
#ifdef WIN32
		closesocket(cobj() );
#else
		close(cobj() );
#endif
		delete data;
	}

	data = other.data;
	data->refcount ++;
	
	return *this;
}

void net6::socket::on_io(condition cond)
{
	data->signal_io.emit(cond);
}

net6::tcp_socket::tcp_socket(const address& addr)
 : socket(address_to_protocol(addr.get_family()), SOCK_STREAM, IPPROTO_TCP)
{
}

net6::tcp_socket::tcp_socket(socket_type c_object)
 : socket(c_object)
{
}

net6::tcp_socket::tcp_socket(const tcp_socket& other)
 : socket(other)
{
}

net6::tcp_socket::~tcp_socket()
{
}

net6::tcp_socket& net6::tcp_socket::operator=(const tcp_socket& other)
{
	return static_cast<tcp_socket&>(socket::operator=(other) );
}

net6::tcp_client_socket::tcp_client_socket(const address& addr)
 : tcp_socket(addr)
{
	if(connect(cobj(), addr.cobj(), addr.get_size()) == -1)
		throw error(net6::error::SYSTEM);
}

net6::tcp_client_socket::tcp_client_socket(socket_type c_object)
 : tcp_socket(c_object)
{
}

net6::tcp_client_socket::tcp_client_socket(const tcp_client_socket& other)
 : tcp_socket(other)
{
}

net6::tcp_client_socket::~tcp_client_socket()
{
}

net6::tcp_client_socket&
net6::tcp_client_socket::operator=(const tcp_client_socket& other)
{
	return static_cast<tcp_client_socket&>(tcp_socket::operator=(other) );
}

net6::socket::size_type
net6::tcp_client_socket::send(const void* buf, size_type len) const
{
	ssize_t result = ::send(cobj(), WIN32_CCAST_FIX(buf), len, 0);
	if(result < 0)
		throw error(net6::error::SYSTEM);

	return result;
}

net6::socket::size_type
net6::tcp_client_socket::recv(void* buf, size_type len) const
{
	ssize_t result = ::recv(cobj(), WIN32_CAST_FIX(buf), len, 0);
	if(result < 0)
		throw error(net6::error::SYSTEM);

	return result;
}

net6::tcp_server_socket::tcp_server_socket(const address& bind_addr)
 : tcp_socket(bind_addr)
{
	if(bind(cobj(), bind_addr.cobj(), bind_addr.get_size()) == -1)
		throw error(net6::error::SYSTEM);
	if(listen(cobj(), 0) == -1)
		throw error(net6::error::SYSTEM);
}

net6::tcp_server_socket::tcp_server_socket(socket_type c_object)
 : tcp_socket(c_object)
{
}

net6::tcp_server_socket::tcp_server_socket(const tcp_server_socket& other)
 : tcp_socket(other)
{
}

net6::tcp_server_socket::~tcp_server_socket()
{
}

net6::tcp_server_socket&
net6::tcp_server_socket::operator=(const tcp_server_socket& other)
{
	return static_cast<tcp_server_socket&>(tcp_socket::operator=(other) );
}

net6::tcp_client_socket net6::tcp_server_socket::accept() const
{
	socket_type new_sock = ::accept(cobj(), NULL, NULL);
	if(new_sock == INVALID_SOCKET)
		throw error(net6::error::SYSTEM);

	return tcp_client_socket(new_sock);
}

net6::tcp_client_socket net6::tcp_server_socket::accept(address& from) const
{
	socklen_t sock_size = from.get_size();
	socket_type new_sock = ::accept(cobj(), from.cobj(), &sock_size);
	if(new_sock == INVALID_SOCKET)
		throw error(net6::error::SYSTEM);

	return tcp_client_socket(new_sock);
}

net6::udp_socket::udp_socket(const address& bind_addr)
 : socket(address_to_protocol(bind_addr.get_family()), SOCK_DGRAM, IPPROTO_UDP)
{
	if(bind(cobj(), bind_addr.cobj(), bind_addr.get_size()) == -1)
		throw error(net6::error::SYSTEM);
}

net6::udp_socket::udp_socket(const udp_socket& other)
 : socket(other)
{
}

net6::udp_socket::~udp_socket()
{
}

net6::udp_socket& net6::udp_socket::operator=(const udp_socket& other)
{
	return static_cast<udp_socket&>(udp_socket::operator=(other) );
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

