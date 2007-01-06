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

#ifndef WIN32
#include <netdb.h>
#include <arpa/inet.h>
#endif

#include <sstream>

#include "common.hpp"
#include "error.hpp"
#include "address.hpp"

namespace
{
#ifdef WIN32
	// We do not use getaddrinfo on WIN32
	hostent* resolve_generic(const char* hostname, int family, int flags)
	{
		hostent* result = gethostbyname(hostname);
		if(result == NULL)
			throw net6::error(
				net6::error::GETHOSTBYNAME,
				WSAGetLastError()
			);

		// Check address type
		if(result->h_addrtype != family)
			throw net6::error(net6::error::GETHOSTBYNAME, NO_DATA);

		return result;
	}
#else
	addrinfo* resolve_generic(const char* hostname, int family, int flags)
	{
		addrinfo hint;
		hint.ai_family = family;
		hint.ai_socktype = 0;
		hint.ai_protocol = 0;
		hint.ai_addrlen = 0;
		hint.ai_addr = NULL;
		hint.ai_canonname = NULL;
		hint.ai_next = NULL;
		hint.ai_flags = flags;

		addrinfo* result;
		int gai_error = getaddrinfo(hostname, NULL, &hint, &result);
		if(gai_error != 0)
			throw net6::error(net6::error::GETADDRINFO, gai_error);

		return result;
	}
#endif

#ifdef WIN32
	// There is no inet_ntop on WIN32
	inline const char *inet_ntop(int af, const void *__restrict src, char *__restrict dest, socklen_t size)
	{
		// IPV6 not supported (yet?)
		if(AF_INET!=af)
		{
			printf(__FUNCTION__ " is only implemented for AF_INET address family on win32/msvc8");
			abort();
		}

		// Format address
		char *s=inet_ntoa(*reinterpret_cast<const in_addr*>(src));
		if(!s)
			return 0;

		// Copy to given buffer
		socklen_t len=(socklen_t)strlen(s);
		if(len>=size)
			return 0;
		return strncpy(dest, s, len);
	}
#endif
}

net6::address::address()
{
}

net6::address::~address()
{
#ifdef DEBUG
	if(addr)
		std::cerr << "Warning: Unfreed address: " << addr << std::endl;
#endif
}

int net6::address::get_family() const
{
	return addr->sa_family;
}

const uint32_t net6::ipv4_address::ANY = INADDR_ANY;
const uint32_t net6::ipv4_address::NONE = INADDR_NONE;
const uint32_t net6::ipv4_address::BROADCAST = INADDR_BROADCAST;
const uint32_t net6::ipv4_address::LOOPBACK = INADDR_LOOPBACK;

net6::ipv4_address::ipv4_address(unsigned int port)
{
	addr = reinterpret_cast<sockaddr*>(new sockaddr_in);
	cobj()->sin_family = AF_INET;
	cobj()->sin_port = htons(port);
	cobj()->sin_addr.s_addr = ANY;
}

net6::ipv4_address net6::ipv4_address::create_from_address(uint32_t ip_address,
                                                           unsigned int port)
{
	ipv4_address addr;
	addr.addr = reinterpret_cast<sockaddr*>(new sockaddr_in);
	addr.cobj()->sin_family = AF_INET;
	addr.cobj()->sin_port = htons(port);
	addr.cobj()->sin_addr.s_addr = ip_address;
	return addr;
}

net6::ipv4_address
net6::ipv4_address::create_from_hostname(const std::string& hostname,
                                         unsigned int port)
{
	ipv4_address addr;
	addr.addr = reinterpret_cast<sockaddr*>(new sockaddr_in);
#ifdef WIN32
	hostent* info = resolve_generic(hostname.c_str(), AF_INET, 0);
#else
	addrinfo* info = resolve_generic(hostname.c_str(), PF_INET, 0);
	sockaddr_in* ai_addr = reinterpret_cast<sockaddr_in*>(info->ai_addr);
#endif

	addr.cobj()->sin_family = AF_INET;
	addr.cobj()->sin_port = htons(port);
#ifdef WIN32
	addr.cobj()->sin_addr.s_addr = *((uint32_t*)info->h_addr_list[0]);
#else
	addr.cobj()->sin_addr.s_addr = ai_addr->sin_addr.s_addr;
	freeaddrinfo(info);
#endif

	return addr;
}

net6::ipv4_address::ipv4_address(const sockaddr_in* other)
 : address()
{
	sockaddr_in* my_addr = new sockaddr_in;
	my_addr->sin_family = other->sin_family;
	my_addr->sin_port = other->sin_port;
	my_addr->sin_addr.s_addr = other->sin_addr.s_addr;
	addr = reinterpret_cast<sockaddr*>(my_addr);
}

net6::ipv4_address::ipv4_address(const ipv4_address& other)
 : address()
{
	sockaddr_in* my_addr = new sockaddr_in;
	sockaddr_in* other_addr = reinterpret_cast<sockaddr_in*>(other.addr);
	my_addr->sin_family = other_addr->sin_family;
	my_addr->sin_port = other_addr->sin_port;
	my_addr->sin_addr.s_addr = other_addr->sin_addr.s_addr;
	addr = reinterpret_cast<sockaddr*>(my_addr);
}

net6::ipv4_address::~ipv4_address()
{
	if(addr)
	{
		delete addr;
		addr = NULL;
	}
}

std::list<net6::ipv4_address>
net6::ipv4_address::list(const std::string& hostname, unsigned int port)
{
	std::list<ipv4_address> result;
#ifdef WIN32
	hostent* info = resolve_generic(hostname.c_str(), AF_INET, 0);
#else
	addrinfo* info = resolve_generic(hostname.c_str(), PF_INET, 0);
#endif

#ifdef WIN32
	for(int cur = 0; cur < info->h_length; ++ cur)
#else
	for(addrinfo* cur = info; cur != NULL; cur = cur->ai_next)
#endif
	{
#ifdef WIN32
		result.push_back(
			ipv4_address::create_from_address(
				*((uint32_t*)info->h_addr_list[cur]),
				port
			)
		);
#else
		sockaddr_in* in = reinterpret_cast<sockaddr_in*>(cur->ai_addr);
		in->sin_port = htons(port);
		result.push_back(ipv4_address(in) );
#endif
	}

#ifndef WIN32
	freeaddrinfo(info);
#endif
	return result;
}

net6::ipv4_address& net6::ipv4_address::operator=(const ipv4_address& other)
{
	if(this != &other) return *this;

	sockaddr_in* my_addr = reinterpret_cast<sockaddr_in*>(addr);
	sockaddr_in* other_addr = reinterpret_cast<sockaddr_in*>(other.addr);

	my_addr->sin_family = other_addr->sin_family;
	my_addr->sin_port = other_addr->sin_port;
	my_addr->sin_addr.s_addr = other_addr->sin_addr.s_addr;

	return *this;
}

net6::ipv4_address& net6::ipv4_address::operator=(const sockaddr_in* other)
{
	sockaddr_in* my_addr = reinterpret_cast<sockaddr_in*>(addr);

	my_addr->sin_family = other->sin_family;
	my_addr->sin_port = other->sin_port;
	my_addr->sin_addr.s_addr = other->sin_addr.s_addr;

	return *this;
}

net6::address* net6::ipv4_address::clone() const
{
	return new ipv4_address(*this);
}

std::string net6::ipv4_address::get_name() const
{
	const size_t len = INET_ADDRSTRLEN;
	char buf[len];

	inet_ntop(AF_INET, &((sockaddr_in*)addr)->sin_addr, buf, len);
	return buf;
}

socklen_t net6::ipv4_address::get_size() const
{
	return sizeof(sockaddr_in);
}

unsigned int net6::ipv4_address::get_port() const
{
	return ntohs(cobj()->sin_port);
}

void net6::ipv4_address::set_port(unsigned int port)
{
	cobj()->sin_port = htons(port);
}

const uint8_t net6::ipv6_address::ANY[16] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const uint8_t net6::ipv6_address::LOOPBACK[16] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1
};

net6::ipv6_address::ipv6_address(unsigned int port, unsigned int flowinfo,
                                 unsigned int scope_id)
{
	addr = reinterpret_cast<sockaddr*>(new sockaddr_in6);
	cobj()->sin6_family = AF_INET6;
	cobj()->sin6_port = htons(port);
	cobj()->sin6_flowinfo = flowinfo;
	std::copy(ANY, ANY + 16, cobj()->sin6_addr.s6_addr);
	cobj()->sin6_scope_id = scope_id;
}

net6::ipv6_address
net6::ipv6_address::create_from_address(const uint8_t ip_address[16],
                                        unsigned int port,
					unsigned int flowinfo,
					unsigned int scope_id)
{
	ipv6_address addr;
	addr.addr = reinterpret_cast<sockaddr*>(new sockaddr_in6);
	addr.cobj()->sin6_family = AF_INET6;
	addr.cobj()->sin6_port = htons(port);
	addr.cobj()->sin6_flowinfo = flowinfo;
	std::copy(ip_address, ip_address + 16, addr.cobj()->sin6_addr.s6_addr);
	addr.cobj()->sin6_scope_id = scope_id;
	return addr;
}

net6::ipv6_address
net6::ipv6_address::create_from_hostname(const std::string& hostname,
                                         unsigned int port,
                                         unsigned int flowinfo,
                                         unsigned int scope_id)
{
	ipv6_address addr;
	addr.addr = reinterpret_cast<sockaddr*>(new sockaddr_in6);
#ifdef WIN32
	hostent* info = resolve_generic(hostname.c_str(), AF_INET6, 0);
#else
	addrinfo* info = resolve_generic(hostname.c_str(), PF_INET6, 0);
	sockaddr_in6* ai_addr = reinterpret_cast<sockaddr_in6*>(info->ai_addr);
#endif

	addr.cobj()->sin6_family = AF_INET6;
	addr.cobj()->sin6_port = htons(port);
	addr.cobj()->sin6_flowinfo = flowinfo;
#ifdef WIN32
	std::copy(
		info->h_addr_list[0],
		info->h_addr_list[0] + 16,
		addr.cobj()->sin6_addr.s6_addr
	);
#else
	std::copy(
		ai_addr->sin6_addr.s6_addr,
		ai_addr->sin6_addr.s6_addr + 16,
	        addr.cobj()->sin6_addr.s6_addr
	);
#endif
	addr.cobj()->sin6_scope_id = scope_id;

#ifndef WIN32
	freeaddrinfo(info);
#endif
	return addr;
}

net6::ipv6_address::ipv6_address(const sockaddr_in6* other)
 : address()
{
	sockaddr_in6* my_addr = new sockaddr_in6;

	my_addr->sin6_family = other->sin6_family;
	my_addr->sin6_port = other->sin6_port;
	my_addr->sin6_flowinfo = other->sin6_flowinfo;
	my_addr->sin6_scope_id = other->sin6_scope_id;
	
	std::copy(
		other->sin6_addr.s6_addr,
		other->sin6_addr.s6_addr + 16,
		my_addr->sin6_addr.s6_addr
	);

	addr = reinterpret_cast<sockaddr*>(my_addr);
}

net6::ipv6_address::ipv6_address(const ipv6_address& other)
 : address()
{
	sockaddr_in6* my_addr = new sockaddr_in6;
	sockaddr_in6* other_addr = reinterpret_cast<sockaddr_in6*>(other.addr);

	my_addr->sin6_family = other_addr->sin6_family;
	my_addr->sin6_port = other_addr->sin6_port;
	my_addr->sin6_flowinfo = other_addr->sin6_flowinfo;
	my_addr->sin6_scope_id = other_addr->sin6_scope_id;
	
	std::copy(
		other_addr->sin6_addr.s6_addr,
		other_addr->sin6_addr.s6_addr + 16,
		my_addr->sin6_addr.s6_addr
	);

	addr = reinterpret_cast<sockaddr*>(my_addr);
}

net6::ipv6_address::~ipv6_address()
{
	if(addr)
	{
		delete addr;
		addr = NULL;
	}
}

std::list<net6::ipv6_address>
net6::ipv6_address::list(const std::string& hostname, unsigned int port,
                         unsigned int flowinfo, unsigned int scope_id)
{
	std::list<ipv6_address> result;
#ifdef WIN32
	hostent* info = resolve_generic(hostname.c_str(), AF_INET6, 0);
#else
	addrinfo* info = resolve_generic(hostname.c_str(), PF_INET6, 0);
#endif

#ifdef WIN32
	for(int cur = 0; cur < info->h_length; ++ cur)
#else
	for(addrinfo* cur = info; cur != NULL; cur = cur->ai_next)
#endif
	{
#ifdef WIN32
		result.push_back(
			ipv6_address::create_from_address(
				reinterpret_cast<uint8_t*>(
					info->h_addr_list[cur]
				),
				port,
				flowinfo,
				scope_id
			)
		);
#else
		sockaddr_in6* in;
		in = reinterpret_cast<sockaddr_in6*>(cur->ai_addr);
		in->sin6_port = htons(port);
		in->sin6_flowinfo = flowinfo;
		in->sin6_scope_id = scope_id;
		result.push_back(ipv6_address(in) );
#endif
	}

#ifndef WIN32
	freeaddrinfo(info);
#endif
	return result;
}

net6::ipv6_address& net6::ipv6_address::operator=(const ipv6_address& other)
{
	if(this != &other) return *this;

	sockaddr_in6* my_addr = reinterpret_cast<sockaddr_in6*>(addr);
	sockaddr_in6* other_addr = reinterpret_cast<sockaddr_in6*>(other.addr);

	my_addr->sin6_family = other_addr->sin6_family;
	my_addr->sin6_port = other_addr->sin6_port;
	my_addr->sin6_flowinfo = other_addr->sin6_flowinfo;
	my_addr->sin6_scope_id = other_addr->sin6_scope_id;
	
	std::copy(
		other_addr->sin6_addr.s6_addr,
		other_addr->sin6_addr.s6_addr + 16,
		my_addr->sin6_addr.s6_addr
	);

	return *this;
}

net6::ipv6_address& net6::ipv6_address::operator=(const sockaddr_in6* other)
{
	sockaddr_in6* my_addr = reinterpret_cast<sockaddr_in6*>(addr);

	my_addr->sin6_family = other->sin6_family;
	my_addr->sin6_port = other->sin6_port;
	my_addr->sin6_flowinfo = other->sin6_flowinfo;
	my_addr->sin6_scope_id = other->sin6_scope_id;
	
	std::copy(
		other->sin6_addr.s6_addr,
		other->sin6_addr.s6_addr + 16,
		my_addr->sin6_addr.s6_addr
	);

	return *this;
}

net6::address* net6::ipv6_address::clone() const
{
	return new ipv6_address(*this);
}

std::string net6::ipv6_address::get_name() const
{
	const size_t len = INET6_ADDRSTRLEN;
	char buf[len];

	inet_ntop(AF_INET6, &((sockaddr_in6*)addr)->sin6_addr, buf, len);
	return buf;
}

socklen_t net6::ipv6_address::get_size() const
{
	return sizeof(sockaddr_in6);
}

unsigned int net6::ipv6_address::get_port() const
{
	return ntohs(cobj()->sin6_port);
}

unsigned int net6::ipv6_address::get_flowinfo() const
{
	return cobj()->sin6_flowinfo;
}

unsigned int net6::ipv6_address::get_scope_id() const
{
	return cobj()->sin6_scope_id;
}

void net6::ipv6_address::set_port(unsigned int port)
{
	cobj()->sin6_port = htons(port);
}

void net6::ipv6_address::set_flowinfo(unsigned int flowinfo)
{
	cobj()->sin6_flowinfo = flowinfo;
}

void net6::ipv6_address::set_scope_id(unsigned int scope_id)
{
	cobj()->sin6_scope_id = scope_id;
}

