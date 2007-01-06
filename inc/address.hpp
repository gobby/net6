/* net6 - library providing ipv4/ipv6 network access
 * Copyright (C) 2005 Armin Burgmeier / 0x539 dev group
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _NET6_ADDRESS_HPP_
#define _NET6_ADDRESS_HPP_

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/ip.h>
#endif

#include <list>
#include <string>

#include <net6/export.hpp>

namespace net6
{

class NET6_EXPORT address
{
public:
	address();
	virtual ~address();

	virtual address* clone() const = 0;

	int get_family() const;
	virtual std::string get_name() const = 0;
	virtual socklen_t get_size() const = 0;

	sockaddr* cobj() { return addr; }
	const sockaddr* cobj() const { return addr; }

protected:
	sockaddr* addr;
};

class NET6_EXPORT ipv4_address : public address
{
public:
	static const uint32_t ANY;
	static const uint32_t NONE;
	static const uint32_t BROADCAST;
	static const uint32_t LOOPBACK;

	static ipv4_address create(unsigned int port = 0);
	static ipv4_address create_from_address(uint32_t ip_address,
	                                        unsigned int port = 0);
	static ipv4_address create_from_hostname(const std::string& hostname,
	                                         unsigned int port = 0);

	ipv4_address(const sockaddr_in* other);
	ipv4_address(const ipv4_address& other);
	virtual ~ipv4_address();

	static std::list<ipv4_address> list(const std::string& hostname,
	                                    unsigned int port = 0);

	ipv4_address& operator=(const ipv4_address& other);
	ipv4_address& operator=(const sockaddr_in* other);

	virtual address* clone() const;

	virtual std::string get_name() const;
	virtual socklen_t get_size() const;

	unsigned int get_port() const;
	void set_port(unsigned int port);

	sockaddr_in* cobj() { return reinterpret_cast<sockaddr_in*>(addr); }
	const sockaddr_in* cobj() const
		{ return reinterpret_cast<sockaddr_in*>(addr); }
protected:
	ipv4_address();
};

class NET6_EXPORT ipv6_address : public address
{
public:
	static const uint8_t ANY[16];
	static const uint8_t LOOPBACK[16];

	static ipv6_address create(unsigned int port = 0,
	                           unsigned int flowinfo = 0,
	                           unsigned int scope_id = 0);
	static ipv6_address create_from_address(const uint8_t ip_address[16],
	                                        unsigned int port = 0,
	                                        unsigned int flowinfo = 0,
	                                        unsigned int scope_id = 0);
	static ipv6_address create_from_hostname(const std::string& hostname,
	                                         unsigned int port = 0,
	                                         unsigned int flowinfo = 0,
	                                         unsigned int scope_id = 0);

	ipv6_address(const sockaddr_in6* other);
	ipv6_address(const ipv6_address& other);
	virtual ~ipv6_address();

	static std::list<ipv6_address> list(const std::string& hostname,
                                            unsigned int port = 0,
	                                    unsigned int flowinfo = 0,
	                                    unsigned int scope_id = 0);

	ipv6_address& operator=(const ipv6_address& other);
	ipv6_address& operator=(const sockaddr_in6* addr);

	virtual address* clone() const;

	virtual std::string get_name() const;
	virtual socklen_t get_size() const;

	unsigned int get_port() const;
	unsigned int get_flowinfo() const;
	unsigned int get_scope_id() const;

	void set_port(unsigned int port);
	void set_flowinfo(unsigned int flowinfo);
	void set_scope_id(unsigned int scope_id);

	sockaddr_in6* cobj() { return reinterpret_cast<sockaddr_in6*>(addr); }
	const sockaddr_in6* cobj() const
		{ return reinterpret_cast<sockaddr_in6*>(addr); }

protected:
	ipv6_address();
};

}

#endif
