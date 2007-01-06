/* net6 - Library providing IPv4/IPv6 network access
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

#ifndef _NET6_CONNECTION_HPP_
#define _NET6_CONNECTION_HPP_

#include <queue>
#include <sigc++/signal.h>

#include "export.hpp"
#include "socket.hpp"
#include "packet.hpp"

namespace net6
{

class NET6_EXPORT connection : public sigc::trackable
{
public:
	// If a close event occurs, the whole connection will be unusable
	// Do not select any longer on the connection!
	typedef sigc::signal<void, const packet&> signal_recv_type;
	typedef sigc::signal<void, const packet&> signal_send_type;
	typedef sigc::signal<void> signal_close_type;

	connection(const address& addr);
	connection(const tcp_client_socket& sock, const address& addr);
	~connection();

	const address& get_remote_address() const;
	const tcp_client_socket& get_socket() const;

	void send(const packet& pack);
	unsigned int send_queue_size() const;

	signal_recv_type recv_event() const;
	signal_send_type send_event() const;
	signal_close_type close_event() const;

protected:
	void on_sock_event(socket& sock, socket::condition io);

	std::queue<packet> packet_queue;
	std::string::size_type offset;
	std::string recv_data;

	signal_recv_type signal_recv;
	signal_send_type signal_send;
	signal_close_type signal_close;

	tcp_client_socket remote_sock;
	address* remote_addr;
};
	
}

#endif

