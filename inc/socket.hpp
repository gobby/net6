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

#ifndef _NET6_SOCKET_HPP_
#define _NET6_SOCKET_HPP_

#include <sigc++/signal.h>

#include "address.hpp"

#ifndef ssize_t
#define ssize_t signed long
#endif

namespace net6
{

/** Abstract socket class. Note that sockets are reference counted.
 */
	
class socket
{
	friend class selector;
public:
	enum condition {
		INCOMING = 0x01,
		OUTGOING = 0x02,
		IOERROR = 0x04
	};

	typedef sigc::signal<void, socket&, condition> signal_read_type;
	typedef sigc::signal<void, socket&, condition> signal_write_type;
	typedef sigc::signal<void, socket&, condition> signal_error_type;

#ifdef WIN32
	typedef SOCKET socket_type;
#else
	typedef int socket_type;
#endif
	typedef size_t size_type;

	/** Create a new reference on a socket
	 */
	socket(const socket& other);
	~socket();

	socket& operator=(const socket& other);

	/** Checks if this socket object references the same object as
	 * <em>other</em>.
	 */
	bool operator==(const socket& other) { return data == other.data; }

	/** Checks if this socket object references another object as
	 * <em>other</em>.
	 */
	bool operator!=(const socket& other) { return data == other.data; }

	/** This signal is emitted if data becomes available for reading.
	 */
	signal_read_type read_event() const { return data->signal_read; }

	/** Signal emitted if data may be written without blocking.
	 */
	signal_write_type write_event() const { return data->signal_write; }

	/** Signal emitted if an error occured on the socket.
	 */
	signal_error_type error_event() const { return data->signal_error; }

	/** Provides access to the underlaying C socket object.
	 */
	socket_type cobj() { return data->sock; }
	const socket_type cobj() const { return data->sock; }
protected:
	socket(int domain, int type, int protocol);
	socket(socket_type c_object);

	struct  socket_data
	{
		socket_type sock;
		int refcount;

		signal_read_type signal_read;
		signal_write_type signal_write;
		signal_error_type signal_error;
	};

	socket_data* data;
};

/** Abstract TCP socket class.
 */

class tcp_socket : public socket
{
public:
	/** Creates a new reference of <em>other</em>.
	 */
	tcp_socket(const tcp_socket& other);
	~tcp_socket();

	tcp_socket& operator=(const tcp_socket& other);

protected:
	tcp_socket(const address& addr);
	tcp_socket(socket_type c_object);
};

/** TCP connection socket.
 */

class tcp_client_socket : public tcp_socket
{
public:
	/** Creates a new tcp socket and connects to the address addr.
	 */
	tcp_client_socket(const address& addr);

	/** Wraps a C socket object. Note that the tcp_client_socket owns
	 * the C object.
	 */
	tcp_client_socket(socket_type c_object);

	/** Creates a new reference of <em>other</em>.
	 */
	tcp_client_socket(const tcp_client_socket& other);
	~tcp_client_socket();

	/** Assigns a new reference of <em>other</em> to this socket object.
	 */
	tcp_client_socket& operator=(const tcp_client_socket& other);

	/** Sends an amount of data through the socket. Note that the call
	 * may block if you did not select on a socket::OUT event.
	 * @return The amount of data sent.
	 */
	size_type send(const void* buf, size_type len) const;

	/** Receives an amount of data from the socket. Note that the call
	 * may block if no data is available.
	 * @return The amount of data read.
	 */
	size_type recv(void* buf, size_type len) const;

protected:
};

/** TCP server socket
 */

class tcp_server_socket : public tcp_socket
{
public:
	/** Opens a new TCP server socket bound to <em>bind_addr</em>.
	 */
	tcp_server_socket(const address& bind_addr);

	/** Wraps a C socket object. Note that the tcp_server_socket owns the
	 * C object.
	 */
	tcp_server_socket(socket_type c_object);

	/** Creates a new reference of <em>other</em>.
	 */
	tcp_server_socket(const tcp_server_socket& other);
	~tcp_server_socket();

	/** Assigns a new reference of <em>other</em> to this socket.
	 */
	tcp_server_socket& operator=(const tcp_server_socket& other);

	/** Accepts a connection from this server socket. Note that the call
	 * blocks until a connection comes available. Selecting on socket::IN
	 * indicates a connection waiting for acception.
	 * @return A tcp_client_socket to communicate with the remote host.
	 */
	tcp_client_socket accept() const;

	/** Accepts a new connection and stores the address of the remote host
	 * in <em>from</em>.
	 */
	tcp_client_socket accept(address& from) const;

protected:
};

/** UDP socket.
 */

class udp_socket : public socket
{
public:
	/** Creates a new UDP socket bound to <em>bind_addr</em>.
	 */
	udp_socket(const address& bind_addr);

	/** Wraps a C UDP socket object.
	 */
	udp_socket(socket_type c_object);

	/** Creates an new reference from <em>other</em>.
	 */
	udp_socket(const udp_socket& other);
	~udp_socket();

	/** Assigns a new reference of <em>other</em> to this UDP socket object.
	 */
	udp_socket& operator=(const udp_socket& other);

	/** Sets the target of this UDP socket. This target is the address to
	 * which datagrams are sent by default and the only address from which
	 * datagrams are received.
	 */
	void set_target(const address& addr);

	/** Resets the target of the UDP socket.
	 */
	void reset_target();

	/** Sends an amount of data to the target of the UDP socket.
	 * @return The amount of data actually sent.
	 */
	size_type send(const void* buf, size_type len) const;

	/** Sends an amount of data to a specified address.
	 * @return The amount of data actually sent.
	 */
	size_type send(const void* bud, size_type len, const address& to) const;

	/** Receives some data from the socket. Note that the call may block
	 * until data becomes available.
	 * @return The amount of data actually read.
	 */
	size_type recv(void* buf, size_type len) const;

	/** Receives some data from the socket and stores the source
	 * address into <em>from</em>. Note that the call may block until
	 * data becomes available for reading.
	 * @return The amount of data actually read.
	 */
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

