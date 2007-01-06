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

#include <sigc++/bind.h>
#include "server.hpp"

net6::server::peer::peer(unsigned int id, const tcp_client_socket& sock,
                         const address& addr)
 : net6::peer(id, ""), logined(false), conn(sock, addr)
{
}

net6::server::peer::~peer()
{
}

void net6::server::peer::login(const std::string& username)
{
	name = username;
	logined = true;
}

bool net6::server::peer::is_logined() const
{
	return logined;
}

void net6::server::peer::send(const packet& pack)
{
	conn.send(pack);
}

unsigned int net6::server::peer::send_queue_size() const
{
	return conn.send_queue_size();
}

const net6::tcp_client_socket& net6::server::peer::get_socket() const
{
	return conn.get_socket();
}

const net6::address& net6::server::peer::get_address() const
{
	return conn.get_remote_address();
}

net6::server::peer::signal_recv_type net6::server::peer::recv_event() const
{
	return conn.recv_event();
}

net6::server::peer::signal_send_type net6::server::peer::send_event() const
{
	return conn.send_event();
}

net6::server::peer::signal_close_type net6::server::peer::close_event() const
{
	return conn.close_event();
}

net6::server::server(bool ipv6)
 : serv_sock(NULL), use_ipv6(ipv6), id_counter(0)
{
}

net6::server::server(unsigned int port, bool ipv6)
 : serv_sock(NULL), use_ipv6(ipv6), id_counter(0)
{
	reopen(port);
}

net6::server::~server()
{
	delete serv_sock;
	
	std::list<peer*>::iterator peer_it;
	for(peer_it = peers.begin(); peer_it != peers.end(); ++ peer_it)
		delete *peer_it;
}

void net6::server::shutdown()
{
	sock_sel.remove(*serv_sock, socket::IN);
	delete serv_sock;
	serv_sock = NULL;
}

void net6::server::reopen(unsigned int port)
{
	if(use_ipv6)
	{
		ipv6_address bind_addr = ipv6_address::create(port);
		serv_sock = new tcp_server_socket(bind_addr);
	}
	else
	{
		ipv4_address bind_addr = ipv4_address::create(port);
		serv_sock = new tcp_server_socket(bind_addr);
	}

	sock_sel.add(*serv_sock, socket::IN);

	serv_sock->read_event().connect(
		sigc::mem_fun(*this, &server::on_server_read) );
/*	serv_sock->error_event().connect(
		sigc::mem_fun(*this, &server::on_server_error) );*/
}

void net6::server::kick(peer& client)
{
	remove_client(&client);
}

void net6::server::select()
{
	sock_sel.select();
}

void net6::server::select(unsigned int timeout)
{
	sock_sel.select(timeout);
}

void net6::server::send(const packet& pack)
{
	std::list<peer*>::const_iterator peer_it;
	for(peer_it = peers.begin(); peer_it != peers.end(); ++ peer_it)
		if( (*peer_it)->is_logined() )
			send(pack, **peer_it);
}

void net6::server::send(const packet& pack, peer& to)
{
	if(to.send_queue_size() == 0)
		sock_sel.add(to.get_socket(), socket::OUT);

	to.send(pack);
}

net6::server::peer* net6::server::find(unsigned int id) const
{
	std::list<peer*>::const_iterator peer_it;
	for(peer_it = peers.begin(); peer_it != peers.end(); ++ peer_it)
		if( (*peer_it)->get_id() == id)
			return *peer_it;
	return NULL;
}

net6::server::peer* net6::server::find(const std::string& name) const
{
	std::list<peer*>::const_iterator peer_it;
	for(peer_it = peers.begin(); peer_it != peers.end(); ++ peer_it)
	{
		if( (*peer_it)->get_name() == name)
			return *peer_it;
	}
	return NULL;
}

net6::server::signal_join_type net6::server::join_event() const
{
	return signal_join;
}

net6::server::signal_part_type net6::server::part_event() const
{
	return signal_part;
}

net6::server::signal_login_type net6::server::login_event() const
{
	return signal_login;
}

net6::server::signal_data_type net6::server::data_event() const
{
	return signal_data;
}

void net6::server::remove_client(peer* client)
{
	signal_part.emit(*client);
	peers.erase(std::remove(peers.begin(), peers.end(), client),
	            peers.end() );

	sock_sel.remove(client->get_socket(), socket::IN | socket::ERR);
	if(sock_sel.check(client->get_socket(), socket::OUT) )
		sock_sel.remove(client->get_socket(), socket::OUT);

	packet pack("net6_client_part");
	pack << static_cast<int>(client->get_id() );
	send(pack);
	delete client;
}

void net6::server::on_server_read(socket& sock, socket::condition io)
{
	address* new_addr;
	if(use_ipv6)
		new_addr = new ipv6_address(ipv6_address::create() );
	else
		new_addr = new ipv4_address(ipv4_address::create() );

	tcp_server_socket& tcp_sock = static_cast<tcp_server_socket&>(sock);
	tcp_client_socket new_sock = tcp_sock.accept(*new_addr);
	peer* new_client = new peer(++id_counter, new_sock, *new_addr);
	delete new_addr;

	peers.push_back(new_client);
	sock_sel.add(new_sock, socket::IN | socket::ERR);

	new_client->recv_event().connect(sigc::bind(
		sigc::mem_fun(*this, &server::on_client_recv),
		sigc::ref(*new_client)
	));

	new_client->send_event().connect(sigc::bind(
		sigc::mem_fun(*this, &server::on_client_send),
		sigc::ref(*new_client)
	));

	new_client->close_event().connect(sigc::bind(
		sigc::mem_fun(*this, &server::on_client_close),
		sigc::ref(*new_client)
	));

	signal_join.emit(*new_client);
}
/*
void net6::server::on_server_error(socket& sock, socket::condition io)
{
	
}
*/
void net6::server::on_client_recv(const packet& pack, peer& from)
{
	if(pack.get_command() == "net6_client_login")
	{
		if(pack.get_param_count() < 1) return;
		if(pack.get_param(0).get_type() != packet::param::STRING)
			return;

		const std::string& name = pack.get_param(0).as_string();
		if(name.empty() )
		{
			packet pack("net6_login_failed");
			pack << "Invalid name";
			send(pack, from);
		}
		else if(find(name) != NULL)
		{
			packet pack("net6_login_failed");
			pack << "Name is already in use";
			send(pack, from);
		}
		else
		{
			from.login(name);
			packet self_pack("net6_client_join");
			self_pack << static_cast<int>(from.get_id() ) << name;
			send(self_pack, from);

			std::list<peer*>::iterator it;
			for(it = peers.begin(); it != peers.end(); ++ it)
			{
				if(!( (*it)->is_logined()) ) continue;
				if(*it == &from) continue;

				packet join_pack("net6_client_join");
				join_pack << static_cast<int>( (*it)->get_id() )
				          << (*it)->get_name();

				send(join_pack, from);
				send(self_pack, **it);
			}

			signal_login.emit(from);
		}
	}
	else
	{
		if(from.is_logined() )
		{
			signal_data.emit(pack, from);
		}
	}
}

void net6::server::on_client_send(const packet& pack, peer& to)
{
	// Do no longer select on socket::OUT if there
	// are no more packets to write
	if(to.send_queue_size() == 0)
		sock_sel.remove(to.get_socket(), socket::OUT);
}

void net6::server::on_client_close(peer& from)
{
	remove_client(&from);
}

