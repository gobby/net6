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

#ifndef _NET6_SOCKET_HPP_
#define _NET6_SOCKET_HPP_

#include <sigc++/signal.h>

#include <net6/export.hpp>
#include <net6/address.hpp>

#ifdef IN
#undef IN
#endif

#ifdef OUT
#undef OUT
#endif

#ifdef ERR
#undef ERR
#endif

#ifndef ssize_t
#define ssize_t signed long
#endif

namespace net6
{

// NOTE: The socket classes are reference counted.
class NET6_EXPORT socket
{
public:
	enum condition {
		IN = 0x01,
		OUT = 0x02,
		ERR = 0x04
	};

/*	class marshaller
	{
	public:
		typedef bool result_type; //OutType;
//		typedef bool OutType;
//		typedef bool InType;

		marshaller() : retval(true) { }
		bool operator()(sigc::internal::slot_iterator_buf<sigc::internal::signal_emit2<bool,net6::socket&, net6::socket::condition, net6::socket::marshaller>, bool> iter1, sigc::internal::slot_iterator_buf<sigc::internal::signal_emit2<bool,net6::socket&, net6::socket::condition, net6::socket::marshaller>, bool> iter2)
		{
			std::cout << "Marshalling: " << std::endl;
			for(;iter1 != iter2; ++ iter1)
			{
				std::cout << "marshalling, iter: " << *iter1 << std::endl;
				if(!(*iter1) )
				{
					return false;
				}
			}

			return true;
		}
	
	protected:
		bool retval;
	};*/
public:
	typedef sigc::signal<void, socket&, condition> signal_read_type;
	typedef sigc::signal<void, socket&, condition> signal_write_type;
	typedef sigc::signal<void, socket&, condition> signal_error_type;

#ifdef WIN32
	typedef SOCKET socket_type;
#else
	typedef int socket_type;
#endif
	typedef size_t size_type;

	socket(const socket& other);
	~socket();

	socket& operator=(const socket& other);

	bool operator==(const socket& other) { return data == other.data; }
	bool operator!=(const socket& other) { return data == other.data; }

	signal_read_type read_event() const { return data->signal_read; }
	signal_write_type write_event() const { return data->signal_write; }
	signal_error_type error_event() const { return data->signal_error; }

	socket_type cobj() { return data->sock; }
	const socket_type cobj() const { return data->sock; }
protected:
	socket(int domain, int type, int protocol);
	socket(socket_type c_object);

	struct NET6_EXPORT socket_data
	{
		socket_type sock;
		int refcount;

		signal_read_type signal_read;
		signal_write_type signal_write;
		signal_error_type signal_error;
	};

	socket_data* data;
};

class NET6_EXPORT tcp_socket : public socket
{
public:
	tcp_socket(const tcp_socket& other);
	~tcp_socket();

	tcp_socket& operator=(const tcp_socket& other);

protected:
	tcp_socket(const address& addr);
	tcp_socket(socket_type c_object);
};

class NET6_EXPORT tcp_client_socket : public tcp_socket
{
public:
	tcp_client_socket(const address& addr);
	tcp_client_socket(socket_type c_object);
	tcp_client_socket(const tcp_client_socket& other);
	~tcp_client_socket();

	tcp_client_socket& operator=(const tcp_client_socket& other);

	size_type send(const void* buf, size_type len) const;
	size_type recv(void* buf, size_type len) const;

protected:
};

class NET6_EXPORT tcp_server_socket : public tcp_socket
{
public:
	tcp_server_socket(const address& bind_addr);
	tcp_server_socket(socket_type c_object);
	tcp_server_socket(const tcp_server_socket& other);
	~tcp_server_socket();

	tcp_server_socket& operator=(const tcp_server_socket& other);

	tcp_client_socket accept() const;
	tcp_client_socket accept(address& from) const;

protected:
};

class NET6_EXPORT udp_socket : public socket
{
public:
	udp_socket(const address& bind_addr);
	udp_socket(socket_type c_object);
	udp_socket(const udp_socket& other);
	~udp_socket();

	udp_socket& operator=(const udp_socket& other);

	void set_target(const address& addr);
	void reset_target();

	size_type send(const void* buf, size_type len) const;
	size_type send(const void* bud, size_type len, const address& to) const;
	size_type recv(void* buf, size_type len) const;
	size_type recv(void* buf, size_type len, address& from) const;
protected:
};

inline socket::condition operator&(socket::condition rhs, socket::condition lhs)
{
	return static_cast<socket::condition>(
		static_cast<int>(rhs) & static_cast<int>(lhs)
	);
}

inline socket::condition operator|(socket::condition rhs, socket::condition lhs)
{
	return static_cast<socket::condition>(
		static_cast<int>(rhs) | static_cast<int>(lhs)
	); 
}

inline socket::condition operator^(socket::condition rhs, socket::condition lhs)
{
	return static_cast<socket::condition>(
		static_cast<int>(rhs) ^ static_cast<int>(lhs)
	); 
}

inline socket::condition& operator&=(socket::condition& rhs, socket::condition lhs)
{
	return rhs = (rhs & lhs);
}

inline socket::condition& operator|=(socket::condition& rhs, socket::condition lhs)
{
	return rhs = (rhs | lhs);
}

inline socket::condition& operator^=(socket::condition& rhs, socket::condition lhs)
{
	return rhs = (rhs ^ lhs);
}

inline socket::condition operator~(socket::condition rhs)
{
	return static_cast<socket::condition>(~static_cast<int>(rhs) );
}

}

#endif

