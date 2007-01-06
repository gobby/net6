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

#ifndef _NET6_SERVER_HPP_
#define _NET6_SERVER_HPP_

#include <sigc++/signal.h>

#include "export.hpp"
#include "peer.hpp"
#include "address.hpp"
#include "socket.hpp"
#include "select.hpp"
#include "packet.hpp"
#include "connection.hpp"

namespace net6
{

class NET6_EXPORT server : public sigc::trackable
{
public:
	class NET6_EXPORT peer : public net6::peer
	{
	public:
		typedef connection::signal_recv_type signal_recv_type;
		typedef connection::signal_send_type signal_send_type;
		typedef connection::signal_close_type signal_close_type;

		peer(unsigned int id, const tcp_client_socket& sock,
		     const address& addr);
		~peer();

		void login(const std::string& username);
		bool is_logined() const;

		void send(const packet& pack);
		unsigned int send_queue_size() const;
		const tcp_client_socket& get_socket() const;
		const address& get_address() const;

		signal_recv_type recv_event() const;
		signal_send_type send_event() const;
		signal_close_type close_event() const;
	protected:
		bool logined;
		connection conn;
	};

	typedef sigc::signal<void, peer&> signal_join_type;
	typedef sigc::signal<void, peer&> signal_part_type;
	typedef sigc::signal<void, peer&> signal_login_type;
	typedef sigc::signal<void, const packet&, peer&> signal_data_type;

	server(bool ipv6 = true);
	server(unsigned int port, bool ipv6 = true);
	~server();

	void shutdown();
	void reopen(unsigned int port);

	void kick(peer& client);
	void select();
	void select(unsigned int timeout);

	void send(const packet& pack);
	void send(const packet& pack, peer& to);

	peer* find(unsigned int id) const;
	peer* find(const std::string& name) const;

	
	signal_join_type join_event() const;
	signal_part_type part_event() const;
	signal_login_type login_event() const;
	signal_data_type data_event() const;

protected:
	void remove_client(peer* client);

	void on_server_read(socket& sock, socket::condition io);
//	void on_server_error(socket& sock, socket::condition io);

	void on_client_recv(const packet& pack, peer& from);
	void on_client_send(const packet& pack, peer& to);
	void on_client_close(peer& from);

	tcp_server_socket* serv_sock;
	std::list<peer*> peers;
	selector sock_sel;
	bool use_ipv6;
	unsigned int id_counter;

	signal_join_type signal_join;
	signal_part_type signal_part;
	signal_login_type signal_login;
	signal_data_type signal_data;
};
	
}

#endif

