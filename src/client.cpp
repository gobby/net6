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

#include <cassert>
#include <sigc++/bind.h>
#include "client.hpp"

net6::client::peer::peer(unsigned int id, const std::string& username)
 : net6::peer(id, username)
{
}

net6::client::peer::~peer()
{
}

/*net6::client::client(const std::string& hostname, unsigned int port, bool ipv6)
 : conn(ipv6 ? ipv6_address::create_from_hostname(hostname, port) :
               ipv6_address::create_from_hostname(hostname, port) ),
   self(NULL)
{
	sock_sel.add(conn.get_socket(), socket::INCOMING | socket::IOERROR);
	conn.recv_event().connect(
		sigc::mem_fun(*this, &client::on_client_recv) );
	conn.close_event().connect(
		sigc::mem_fun(*this, &client::on_client_close) );
}*/

net6::client::client(const address& addr)
 : conn(addr), self(NULL)
{
	sock_sel.add(conn.get_socket(), socket::INCOMING | socket::IOERROR);
	conn.send_event().connect(
		sigc::mem_fun(*this, &client::on_send_event) );
	conn.recv_event().connect(
		sigc::mem_fun(*this, &client::on_recv_event) );
	conn.close_event().connect(
		sigc::mem_fun(*this, &client::on_close_event) );
}

net6::client::~client()
{
	std::list<peer*>::iterator peer_it;
	for(peer_it = peers.begin(); peer_it != peers.end(); ++ peer_it)
		delete *peer_it;
}

void net6::client::login(const std::string& username)
{
	packet login_pack("net6_client_login");
	login_pack << username;
	send(login_pack);
}

void net6::client::custom_login(const packet& pack)
{
	assert(pack.get_command() == "net6_client_login");
	assert(pack.get_param_count() >= 1);
	assert(pack.get_param(0).get_type() == packet::param::STRING);

	send(pack);
}

void net6::client::select()
{
	sock_sel.select();
}

void net6::client::select(unsigned int timeout)
{
	sock_sel.select(timeout);
}

void net6::client::send(const packet& pack)
{
	if(conn.send_queue_size() == 0)
		sock_sel.add(conn.get_socket(), socket::OUTGOING);
	conn.send(pack);
}

net6::client::peer* net6::client::find(unsigned int id) const
{
	std::list<peer*>::const_iterator peer_it;
	for(peer_it = peers.begin(); peer_it != peers.end(); ++ peer_it)
		if( (*peer_it)->get_id() == id)
			return *peer_it;
	return NULL;
}

net6::client::peer* net6::client::find(const std::string& name) const
{
	std::list<peer*>::const_iterator peer_it;
	for(peer_it = peers.begin(); peer_it != peers.end(); ++ peer_it)
		if( (*peer_it)->get_name() == name)
			return *peer_it;
	return NULL;
}

net6::client::peer* net6::client::get_self() const
{
	return self;
}

const net6::connection& net6::client::get_connection() const
{
	return conn;
}

net6::client::signal_join_type net6::client::join_event() const
{
	return signal_join;
}

net6::client::signal_part_type net6::client::part_event() const
{
	return signal_part;
}

net6::client::signal_data_type net6::client::data_event() const
{
	return signal_data;
}

net6::client::signal_close_type net6::client::close_event() const
{
	return signal_close;
}

net6::client::signal_login_failed_type net6::client::login_failed_event() const
{
	return signal_login_failed;
}

void net6::client::on_recv_event(const packet& pack)
{
	if(pack.get_command() == "net6_login_failed")
		net_login_failed(pack);
	else if(pack.get_command() == "net6_client_join")
		net_client_join(pack);
	else if(pack.get_command() == "net6_client_part")
		net_client_part(pack);
	else
		on_data(pack);
}

void net6::client::on_send_event(const packet& pack)
{
	if(conn.send_queue_size() == 0)
		sock_sel.remove(conn.get_socket(), socket::OUTGOING);
}

void net6::client::on_close_event()
{
	on_close();
}

void net6::client::on_join(peer& client, const packet& pack)
{
	signal_join.emit(client, pack);
}

void net6::client::on_part(peer& client, const packet& pack)
{
	signal_part.emit(client, pack);
}

void net6::client::on_data(const packet& pack)
{
	signal_data.emit(pack);
}

void net6::client::on_close()
{
	signal_close.emit();
}

void net6::client::on_login_failed(const std::string& reason)
{
	signal_login_failed.emit(reason);
}

void net6::client::net_login_failed(const packet& pack)
{
	if(pack.get_param_count() < 1) return;
	if(pack.get_param(0).get_type() != packet::param::STRING) return;

	on_login_failed(pack.get_param(0).as_string() );
}

void net6::client::net_client_join(const packet& pack)
{
	if(pack.get_param_count() < 2) return;
	if(pack.get_param(0).get_type() != packet::param::INT) return;
	if(pack.get_param(1).get_type() != packet::param::STRING) return;

	int id = pack.get_param(0).as_int();
	const std::string& name = pack.get_param(1).as_string();

	peer* new_client = new peer(id, name);
	peers.push_back(new_client);

	// The first client who joins is the client representing this host.
	if(!self) self = new_client;
	on_join(*new_client, pack);
}

void net6::client::net_client_part(const packet& pack)
{
	if(pack.get_param_count() < 1) return;
	if(pack.get_param(0).get_type() != packet::param::INT) return;

	int id = pack.get_param(0).as_int();
	peer* rem_peer = find(id);
	
	if(!rem_peer) return;
	on_part(*rem_peer, pack);

	peers.erase(std::remove(peers.begin(), peers.end(), rem_peer),
	            peers.end() );
	delete rem_peer;
}

