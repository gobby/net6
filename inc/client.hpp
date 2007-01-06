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

#ifndef _NET6_CLIENT_HPP_
#define _NET6_CLIENT_HPP_

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

class NET6_EXPORT client : public sigc::trackable
{
public:
	class NET6_EXPORT peer : public net6::peer
	{
	public:
		peer(unsigned int id, const std::string& username);
		~peer();

	protected:
	};

	typedef sigc::signal<void, peer&> signal_join_type;
	typedef sigc::signal<void, peer&> signal_part_type;
	typedef sigc::signal<void, const packet&> signal_data_type;
	typedef sigc::signal<void> signal_close_type;
	typedef sigc::signal<void, const std::string&> signal_login_failed_type;

//	client(const std::string& hostname, unsigned int port,
//	       bool ipv6 = true);
	client(const address& addr);
	~client();

	void login(const std::string& username);

	void select();
	void select(unsigned int timeout);
	void send(const packet& pack);

	peer* find(unsigned int id) const;
	peer* find(const std::string& name) const;
	peer* get_self() const;
	
	signal_join_type join_event() const;
	signal_part_type part_event() const;
	signal_data_type data_event() const;
	signal_close_type close_event() const;
	signal_login_failed_type login_failed_event() const;

protected:
	void on_client_recv(const packet& pack);
	void on_client_send(const packet& pack);
	void on_client_close();

	void on_login_failed(const packet& pack);
	void on_client_join(const packet& pack);
	void on_client_part(const packet& pack);

	connection conn;
	std::list<peer*> peers;
	selector sock_sel;
	peer* self;

	signal_join_type signal_join;
	signal_part_type signal_part;
	signal_data_type signal_data;
	signal_close_type signal_close;
	signal_login_failed_type signal_login_failed;
};
	
}

#endif

