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

#include <memory>
#include <sigc++/signal.h>

#include "enum_ops.hpp"
#include "non_copyable.hpp"
#include "address.hpp"

#ifndef ssize_t
#define ssize_t signed long
#endif

namespace net6
{

enum io_condition
{
	IO_NONE     = 0x00,
	IO_INCOMING = 0x01,
	IO_OUTGOING = 0x02,
	IO_ERROR    = 0x04
};

NET6_DEFINE_ENUM_OPS(io_condition)

/** Abstract socket class.
 */
class socket: private non_copyable
{
public:
	typedef sigc::signal<void, io_condition> signal_io_type;

#ifdef WIN32
	typedef SOCKET socket_type;
#else
	typedef int socket_type;
#endif
	typedef size_t size_type;

	/** @brief Closes the socket.
	 */
	~socket();

	/** Signal which will be emitted if somehting occures with the socket.
	 */
	signal_io_type io_event() const { return signal_io; }

	/** Provides access to the underlaying C socket object.
	 */
	socket_type cobj() { return sock; }
	const socket_type cobj() const { return sock; }
protected:
	socket(int domain, int type, int protocol);
	socket(socket_type c_object);

private:
	socket_type sock;
	signal_io_type signal_io;
};

/** Abstract TCP socket class.
 */
class tcp_socket: public socket
{
protected:
	tcp_socket(const address& addr);
	tcp_socket(socket_type c_object);
};

/** TCP connection socket.
 */

class tcp_client_socket: public tcp_socket
{
public:
	/** Creates a new tcp socket and connects to the address addr.
	 */
	tcp_client_socket(const address& addr);

	/** Wraps a C socket object. Note that the tcp_client_socket owns
	 * the C object.
	 */
	tcp_client_socket(socket_type c_object);

	/** Sends an amount of data through the socket. Note that the call
	 * may block if you did not select on a socket::OUT event.
	 * @return The amount of data sent.
	 */
	size_type send(const void* buf,
	               size_type len) const;

	/** Receives an amount of data from the socket. Note that the call
	 * may block if no data is available.
	 * @return The amount of data read.
	 */
	size_type recv(void* buf,
	               size_type len) const;
};

/** TCP server socket
 */

class tcp_server_socket: public tcp_socket
{
public:
	/** Opens a new TCP server socket bound to <em>bind_addr</em>.
	 */
	tcp_server_socket(const address& bind_addr);

	/** Wraps a C socket object. Note that the tcp_server_socket owns the
	 * C object.
	 */
	tcp_server_socket(socket_type c_object);

	/** Accepts a connection from this server socket. Note that the call
	 * blocks until a connection comes available. Selecting on socket::IN
	 * indicates a connection waiting for acception.
	 * @return A tcp_client_socket to communicate with the remote host.
	 */
	std::auto_ptr<tcp_client_socket> accept() const;

	/** Accepts a new connection and stores the address of the remote host
	 * in <em>from</em>.
	 */
	std::auto_ptr<tcp_client_socket> accept(address& from) const;
};

/** UDP socket.
 */
class udp_socket: public socket
{
public:
	/** Creates a new UDP socket bound to <em>bind_addr</em>.
	 */
	udp_socket(const address& bind_addr);

	/** Wraps a C UDP socket object.
	 */
	udp_socket(socket_type c_object);

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
	size_type send(const void* buf,
	               size_type len) const;

	/** Sends an amount of data to a specified address.
	 * @return The amount of data actually sent.
	 */
	size_type send(const void* bud,
	               size_type len,
	               const address& to) const;

	/** Receives some data from the socket. Note that the call may block
	 * until data becomes available.
	 * @return The amount of data actually read.
	 */
	size_type recv(void* buf,
	               size_type len) const;

	/** Receives some data from the socket and stores the source
	 * address into <em>from</em>. Note that the call may block until
	 * data becomes available for reading.
	 * @return The amount of data actually read.
	 */
	size_type recv(void* buf,
	               size_type len,
	               address& from) const;
};

} // namespace net6

#endif // _NET6_SOCKET_HPP_

