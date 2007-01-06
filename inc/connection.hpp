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

#ifndef _NET6_CONNECTION_HPP_
#define _NET6_CONNECTION_HPP_

#include <memory>
#include <sigc++/signal.h>

#include "non_copyable.hpp"
#include "socket.hpp"
#include "queue.hpp"
#include "packet.hpp"

namespace net6
{

/** Abstract base connection class. Instantiate net6::connection.
 */
class connection_base: public sigc::trackable, private non_copyable
{
public:
	enum conn_state {
		UNENCRYPTED,
		ENCRYPTION_INITIATED_CLIENT,
		ENCRYPTION_INITIATED_SERVER,
		ENCRYPTION_HANDSHAKING,
		ENCRYPTED,
		CLOSED
	};

	typedef sigc::signal<void, const packet&> signal_recv_type;
	typedef sigc::signal<void> signal_send_type;
	typedef sigc::signal<void> signal_close_type;
	typedef sigc::signal<void> signal_encrypted_type;

	/** @brief Creates a new connection that is initially in closed
	 * state.
	 */
	connection_base();

	virtual ~connection_base();

	/** @brief Connects to the given address if the connection is closed.
	 */
	void connect(const address& addr);

	/** @brief Wraps the given socket into this connection.
	 */
	void assign(std::auto_ptr<tcp_client_socket> sock,
	            const address& addr);

	/** Returns the remote internet address.
	 */
	const address& get_remote_address() const;

	/** Returns the underlaying TCP socket object.
	 */
	const tcp_client_socket& get_socket() const;

	/** Queues a packet to send it to the remote host.
	 */
	void send(const packet& pack);

	/** @brief Requests a secure connection to the remote end.
	 *
	 * signal_encrypted will be emitted when further traffic will be
	 * encrypted.
	 */
	void request_encryption(bool as_client);

	/** Signal which is emitted when a packet has been received.
	 */
	signal_recv_type recv_event() const;

	/** Signal which is emitted when the connection has been lost. Note
	 * that the connection is invalid after the close event occured!
	 */
	signal_close_type close_event() const;

	/** Signal which is emitted when the connection is guaranteed to
	 * be encrypted.
	 */
	signal_encrypted_type encrypted_event() const;

protected:
	virtual void set_select(io_condition cond) = 0;
	virtual io_condition get_select() const = 0;

	void on_recv(const packet& pack);
	void on_send();
	void on_close();

	queue sendqueue;
	queue recvqueue;

	signal_recv_type signal_recv;
	signal_close_type signal_close;
	signal_encrypted_type signal_encrypted;

	std::auto_ptr<tcp_client_socket> remote_sock;
	tcp_encrypted_socket_base* encrypted_sock;
	std::auto_ptr<address> remote_addr;

	conn_state state;

private:
	void setup_signal();
	void init_impl();

	void on_sock_event(io_condition io);
	void do_io(io_condition io);

	void do_recv(const packet& pack);
	void do_handshake();

	void net_encryption(const packet& pack);
	void net_encryption_ok(const packet& pack);
	void net_encryption_failed(const packet& pack);
};

/** @brief Connection to another host.
 */
template<typename Selector>
class connection: public connection_base
{
public:
	typedef Selector selector_type;

	connection(selector_type& sel);

	virtual ~connection();

protected:
	virtual void set_select(io_condition cond);
	virtual io_condition get_select() const;

	selector_type& selector;
};

template<typename Selector>
connection<Selector>::connection(selector_type& sel):
	selector(sel)
{
}

template<typename Selector>
connection<Selector>::~connection()
{
	// TODO: Should be done by connection_base dtor?
	selector.set(*remote_sock, IO_NONE);
}

template<typename Selector>
void connection<Selector>::set_select(io_condition cond)
{
	selector.set(*remote_sock, cond);
}

template<typename Selector>
io_condition connection<Selector>::get_select() const
{
	return selector.get(*remote_sock);
}

} // namespace net6

#endif // _NET6_CONNECTION_HPP_
