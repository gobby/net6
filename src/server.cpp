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

#include <sigc++/bind.h>
#include "server.hpp"

net6::server::peer::peer(unsigned int id)
 : net6::peer(id, ""), logged_in(false), conn(NULL)
{
}

net6::server::peer::peer(unsigned int id, const tcp_client_socket& sock,
                         const address& addr)
 : net6::peer(id, ""), logged_in(false), conn(new net6::connection(sock, addr) )
{
}

net6::server::peer::~peer()
{
	delete conn;
}

void net6::server::peer::login(const std::string& username)
{
	name = username;
	logged_in = true;
}

bool net6::server::peer::is_logged_in() const
{
	return logged_in;
}

void net6::server::peer::send(const packet& pack)
{
	conn->send(pack);
}

const net6::tcp_client_socket& net6::server::peer::get_socket() const
{
	return conn->get_socket();
}

const net6::address& net6::server::peer::get_address() const
{
	return conn->get_remote_address();
}

net6::server::peer::signal_send_type net6::server::peer::send_event() const
{
	return conn->send_event();
}

net6::server::peer::signal_recv_type net6::server::peer::recv_event() const
{
	return conn->recv_event();
}

net6::server::peer::signal_close_type net6::server::peer::close_event() const
{
	return conn->close_event();
}

net6::server::server(bool ipv6)
 : serv_sock(NULL), use_ipv6(ipv6), id_counter(0)
{
}

net6::server::server(unsigned int port, bool ipv6)
 : serv_sock(NULL), use_ipv6(ipv6), id_counter(0)
{
	reopen_impl(port);
}

net6::server::~server()
{
	// Shutdown server socket
	shutdown_impl();

	// Delete clients
	std::list<peer*>::iterator peer_it;
	for(peer_it = peers.begin(); peer_it != peers.end(); ++ peer_it)
		delete *peer_it;
	peers.clear();
}

void net6::server::shutdown()
{
	shutdown_impl();
}

void net6::server::reopen(unsigned int port)
{
	reopen_impl(port);
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
		if( (*peer_it)->is_logged_in() )
			send(pack, **peer_it);
}

void net6::server::send(const packet& pack, peer& to)
{
	// Select for outgoing data if we do not already
	if(!sock_sel.check(to.get_socket(), socket::OUTGOING) )
		sock_sel.add(to.get_socket(), socket::OUTGOING);

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

const net6::tcp_server_socket& net6::server::get_socket() const
{
	return *serv_sock;
}

net6::server::signal_connect_type net6::server::connect_event() const
{
	return signal_connect;
}

net6::server::signal_disconnect_type net6::server::disconnect_event() const
{
	return signal_disconnect;
}

net6::server::signal_join_type net6::server::join_event() const
{
	return signal_join;
}

net6::server::signal_part_type net6::server::part_event() const
{
	return signal_part;
}

net6::server::signal_login_auth_type net6::server::login_auth_event() const
{
	return signal_login_auth;
}

net6::server::signal_login_type net6::server::login_event() const
{
	return signal_login;
}

net6::server::signal_login_extend_type net6::server::login_extend_event() const
{
	return signal_login_extend;
}

net6::server::signal_data_type net6::server::data_event() const
{
	return signal_data;
}

void net6::server::remove_client(peer* client)
{
	if(client->is_logged_in() )
		on_part(*client);
	on_disconnect(*client);

	peers.erase(std::remove(peers.begin(), peers.end(), client),
	            peers.end() );

	sock_sel.remove(
		client->get_socket(),
		socket::INCOMING | socket::IOERROR
	);

	if(sock_sel.check(client->get_socket(), socket::OUTGOING) )
		sock_sel.remove(client->get_socket(), socket::OUTGOING);

	packet pack("net6_client_part");
	pack << client->get_id();
	send(pack);
	delete client;
}

void net6::server::on_accept_event(socket::condition io)
{
	peer* new_client = NULL;
	if(use_ipv6)
	{
		ipv6_address addr;
		tcp_client_socket new_sock = serv_sock->accept(addr);
		new_client = new peer(++id_counter, new_sock, addr);
	}
	else
	{
		ipv4_address addr;
		tcp_client_socket new_sock = serv_sock->accept(addr);
		new_client = new peer(++id_counter, new_sock, addr);
	}

	peers.push_back(new_client);
	sock_sel.add(
		new_client->get_socket(),
		socket::INCOMING | socket::IOERROR
	);

	new_client->send_event().connect(sigc::bind(
		sigc::mem_fun(*this, &server::on_send_event),
		sigc::ref(*new_client)
	));

	new_client->recv_event().connect(sigc::bind(
		sigc::mem_fun(*this, &server::on_recv_event),
		sigc::ref(*new_client)
	));

	new_client->close_event().connect(sigc::bind(
		sigc::mem_fun(*this, &server::on_close_event),
		sigc::ref(*new_client)
	));

	on_connect(*new_client);
}

void net6::server::on_send_event(peer& to)
{
	// No more packets to write: Do not select for outgoing data anymore
	sock_sel.remove(to.get_socket(), socket::OUTGOING);
}

void net6::server::on_recv_event(const packet& pack, peer& from)
{
	if(pack.get_command() == "net6_client_login")
		net_client_login(from, pack);
	else
		if(from.is_logged_in() )
			on_data(from, pack);
}

void net6::server::on_close_event(peer& from)
{
	remove_client(&from);
}

void net6::server::on_connect(peer& client)
{
	signal_connect.emit(client);
}

void net6::server::on_disconnect(peer& client)
{
	signal_disconnect.emit(client);
}

void net6::server::on_join(peer& new_peer)
{
	signal_join.emit(new_peer);
}

void net6::server::on_part(peer& client)
{
	signal_part.emit(client);
}

bool net6::server::on_login_auth(peer& client, const packet& pack,
                                 login::error& error)
{
	return signal_login_auth.emit(client, pack, error);
}

unsigned int net6::server::on_login(peer& client, const packet& pack)
{
	return signal_login.emit(client, pack);
}

void net6::server::on_login_extend(peer& client, packet& pack)
{
	signal_login_extend.emit(client, pack);
}

void net6::server::on_data(peer& client, const packet& pack)
{
	signal_data.emit(client, pack);
}

void net6::server::net_client_login(peer& from, const packet& pack)
{
	// Is already logged in
	if(from.is_logged_in() ) return;

	// Get wished user name
	// TODO: trim name?
	const std::string& name = pack.get_param(0).as<std::string>();

	// Check for valid user name
	if(name.empty() )
	{
		packet pack("net6_login_failed");
		pack << static_cast<int>(login::ERROR_NAME_INVALID);
		send(pack, from);
	}
	// Check for existing user name
	else if(find(name) != NULL)
	{
		packet pack("net6_login_failed");
		pack << static_cast<int>(login::ERROR_NAME_IN_USE);
		send(pack, from);
	}
	else
	{
		// Check for login_auth
		login::error reason;
		if(!on_login_auth(from, pack, reason) )
		{
			packet pack("net6_login_failed");
			pack << reason;
			send(pack, from);
			return;
		}

		// Login succeeded
		from.login(name);
		unsigned int new_id = on_login(from, pack);

		if(new_id != 0)
		{
			from.id = new_id;

			if(id_counter < new_id)
				id_counter = new_id;
		}

		// Synchronise with other clients
		packet self_pack("net6_client_join");
		self_pack << from.get_id() << name;
		on_login_extend(from, self_pack);
		send(self_pack, from);

		std::list<peer*>::iterator it;
		for(it = peers.begin(); it != peers.end(); ++ it)
		{
			if(!( (*it)->is_logged_in()) ) continue;
			if(*it == &from) continue;

			packet join_pack("net6_client_join");
			join_pack << (*it)->get_id() << (*it)->get_name();
			on_login_extend(**it, join_pack);

			send(join_pack, from);
			send(self_pack, **it);
		}

		// Join complete
		on_join(from);
	}
}

void net6::server::shutdown_impl()
{
	sock_sel.remove(*serv_sock, socket::INCOMING);
	delete serv_sock;
	serv_sock = NULL;
}

void net6::server::reopen_impl(unsigned int port)
{
	if(use_ipv6)
	{
		ipv6_address bind_addr(port);
		serv_sock = new tcp_server_socket(bind_addr);
	}
	else
	{
		ipv4_address bind_addr(port);
		serv_sock = new tcp_server_socket(bind_addr);
	}

	sock_sel.add(*serv_sock, socket::INCOMING);
	serv_sock->io_event().connect(
		sigc::mem_fun(*this, &server::on_accept_event) );
}

