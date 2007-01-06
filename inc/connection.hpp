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

#ifndef _NET6_CONNECTION_HPP_
#define _NET6_CONNECTION_HPP_

#include <sigc++/signal.h>

#include "non_copyable.hpp"
#include "socket.hpp"
#include "packet.hpp"

namespace net6
{

/** Connection to another host.
 */

class connection : public sigc::trackable, private non_copyable
{
public:
	/** Internal buffer for incoming or outgoing data.
	 */
	class queue : private non_copyable
	{
	public:
		typedef size_t size_type;

		queue();
		~queue();

		/** Returns the size of the queue.
		 */
		size_type get_size() const;

		/** Returns the size of the next packet in the queue.
		 */
		size_type packet_size() const;

		/** Returns a pointer to the data that is currently enqueued.
		 */
		const char* get_data() const;

		/** Appends new data to the queue.
		 */
		void append(const char* new_data, size_type len);

		/** Removes data from the queue.
		 */
		void remove(size_type len);

	private:
		char* data;
		size_t size;
		size_t alloc;
	};

	typedef sigc::signal<void, const packet&> signal_recv_type;
	typedef sigc::signal<void> signal_send_type;
	typedef sigc::signal<void> signal_close_type;

	/** Creates a new connection object and establishes a connection to
	 * <em>addr</em>.
	 */
	connection(const address& addr);

	/** Wraps a socket a to a connection object. The remote host has the
	 * address <em>addr</em>.
	 */
	connection(const tcp_client_socket& sock, const address& addr);
	~connection();

	/** Returns the remote internet address.
	 */
	const address& get_remote_address() const;

	/** Returns the underlaying TCP socket object.
	 */
	const tcp_client_socket& get_socket() const;

	/** Queues a packet to send it to the remote host. Note that the
	 * socket has to be selected for socket::OUTGOING, if packets
	 * have to be sent.
	 */
	void send(const packet& pack);

	/** Returns the amount of packets queued for sending.
	 */
	//unsigned int send_queue_size() const;

	/** Signal which is emitted when a packet has been received. Note that
	 * the underlaying socket has to be selected for socket::IN to
	 * receive packets.
	 */
	signal_recv_type recv_event() const;

	/** Signal which is emitted when all data to be sent has been sent.
	 * This is a good place to remove a socket::OUTGOING flag for the
	 * underlaying socket.
	 */
	signal_send_type send_event() const;

	/** Signal which is emitted when the connection has been lost. Note
	 * that the connection is invalid after the close event occured!
	 */
	signal_close_type close_event() const;

protected:
	void on_sock_event(socket::condition io);

	void on_send();
	void on_recv(const net6::packet& pack);
	void on_close();

	queue sendqueue;
	queue recvqueue;

	signal_recv_type signal_recv;
	signal_send_type signal_send;
	signal_close_type signal_close;

	tcp_client_socket remote_sock;
	address* remote_addr;
};
	
}

#endif

