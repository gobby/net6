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

#ifndef _NET6_CLIENT_HPP_
#define _NET6_CLIENT_HPP_

#include <memory>
#include <sigc++/signal.h>

#include "error.hpp"
#include "user.hpp"
#include "address.hpp"
#include "socket.hpp"
#include "select.hpp"
#include "packet.hpp"
#include "connection.hpp"
#include "local.hpp"

namespace net6
{

/** Client in a Client/Server based TCP network.
 */
template<typename selector_type>
class basic_client: virtual public basic_local<selector_type>
{
public:
	typedef sigc::signal<void, const user&, const packet&>
		signal_join_type;
	typedef sigc::signal<void, const user&, const packet&>
		signal_part_type;
	typedef sigc::signal<void, const packet&>
		signal_data_type;
	typedef sigc::signal<void>
		signal_close_type;
	typedef sigc::signal<void, login::error>
		signal_login_failed_type;
	typedef sigc::signal<void, packet&>
		signal_login_extend_type;

	/** Creates a new basic_client which is not connected to anywhere.
	 */
	basic_client();

	/** Creates a new basic_client and connect to the server at
	 * <em>addr</em>.
	 */
	basic_client(const address& addr);

	/** Destructor disconnects from the server, if the client is
	 * connected.
	 */
	virtual ~basic_client();

	/** Connect to the given address. Only use this if the client is not
	 * already connected.
	 */
	virtual void connect(const address& addr);

	/** Disconnects from the server.
	 */
	virtual void disconnect();

	/** Determinates if the client is connected to a server.
	 */
	bool is_connected() const;

	/** Send a login request with the specified user name. On success,
	 * a join_event with user==self is emitted, otherwise a
	 * login_failed_event.
	 */
	void login(const std::string& username);

	/** Send a packet to the network server.
	 */
	virtual void send(const packet& pack);

	/** Returns the user object which represents the
	 * local host in the network.
	 */
	virtual user& get_self();

	/** Returns the user object which represents the
	 * local host in the network.
	 */
	virtual const user& get_self() const;

	/** Returns the underlaying net6::connection object.
	 */
	const connection& get_connection() const;
	
	/** Signal which is emitted every time a client joins the network.
	 */
	signal_join_type join_event() const;

	/** Signal which is emitted every time a client parts the network.
	 */
	signal_part_type part_event() const;

	/** Signal which is emitted when a packet from the server arrived.
	 */
	signal_data_type data_event() const;

	/** Signal which is emitted when the connection to the server has
	 * been lost. The client will end up in disconnected state after having
	 * received this event.
	 */
	signal_close_type close_event() const;

	/** Signal which is emitted, if a login request failed, for example
	 * if the wished user name was already in use by another client.
	 */
	signal_login_failed_type login_failed_event() const;

	/** Signal which is emitted when a login packet will be sent (most
	 * likely by a call to basic_client::login). It allows the user
	 * to append some more parameters to the login packet which may be
	 * evaluated by the server object in its login_auth and login signal
	 * handlers.
	 */
	signal_login_extend_type login_extend_event() const;

protected:
	/** Signal handler that is called each time a packet arrives.
	 */
	virtual void on_recv_event(const packet& pack);

	/** Signal handler that is called when all available data has been sent.
	 */
	virtual void on_send_event();

	/** Signal handler that is called when the remote site closed the
	 * connection.
	 */
	virtual void on_close_event();

	virtual void on_join(const user& user, const packet& pack);
	virtual void on_part(const user& user, const packet& pack);
	virtual void on_data(const packet& pack);
	virtual void on_close();
	virtual void on_login_failed(login::error error);
	virtual void on_login_extend(packet& pack);

	virtual void net_login_failed(const packet& pack);
	virtual void net_client_join(const packet& pack);
	virtual void net_client_part(const packet& pack);

	std::auto_ptr<connection> conn;
	user* self;

	signal_join_type signal_join;
	signal_part_type signal_part;
	signal_data_type signal_data;
	signal_close_type signal_close;
	signal_login_failed_type signal_login_failed;
	signal_login_extend_type signal_login_extend;

private:
	void connect_impl(const address& addr);
	void disconnect_impl();
};

typedef basic_client<selector> client;

template<typename selector_type>
basic_client<selector_type>::basic_client():
	basic_local<selector_type>(), self(NULL)
{
}

template<typename selector_type>
basic_client<selector_type>::basic_client(const net6::address& addr):
	basic_local<selector_type>(), self(NULL)
{
	connect_impl(addr);
}

template<typename selector_type>
basic_client<selector_type>::~basic_client()
{
	if(is_connected() )
		disconnect_impl();
}

template<typename selector_type>
void basic_client<selector_type>::connect(const net6::address& addr)
{
	connect_impl(addr);
}

template<typename selector_type>
void basic_client<selector_type>::disconnect()
{
	disconnect_impl();
}

template<typename selector_type>
bool basic_client<selector_type>::is_connected() const
{
	return conn.get() != NULL;
}

template<typename selector_type>
void basic_client<selector_type>::login(const std::string& username)
{
	packet login_pack("net6_client_login");
	login_pack << username;
	on_login_extend(login_pack);
	send(login_pack);
}

template<typename selector_type>
void basic_client<selector_type>::send(const packet& pack)
{
	selector_type& selector = basic_object<selector_type>::get_selector();

	// Add OUTGOING flag it to the selector if it isn't already set
	if(selector.check(conn->get_socket(), IO_OUTGOING) == IO_NONE)
		selector.add(conn->get_socket(), IO_OUTGOING);

	// Add packet to send queue
	conn->send(pack);
}

template<typename selector_type>
user& basic_client<selector_type>::get_self()
{
	if(self == NULL)
		// TODO: not_logged_in_error?
		throw not_connected_error("net6::basic_client::get_self");

	return *self;
}

template<typename selector_type>
const user& basic_client<selector_type>::get_self() const
{
	if(self == NULL)
		// TODO: not_logged_in_error?
		throw not_connected_error("net6::basic_client::get_self");

	return *self;
}

template<typename selector_type>
const connection& basic_client<selector_type>::get_connection() const
{
	if(!is_connected() )
		throw not_connected_error("net6::basic_client::get_connection");

	return *conn;
}

template<typename selector_type>
typename basic_client<selector_type>::signal_join_type
basic_client<selector_type>::join_event() const
{
	return signal_join;
}

template<typename selector_type>
typename basic_client<selector_type>::signal_part_type
basic_client<selector_type>::part_event() const
{
	return signal_part;
}

template<typename selector_type>
typename basic_client<selector_type>::signal_data_type
basic_client<selector_type>::data_event() const
{
	return signal_data;
}

template<typename selector_type>
typename basic_client<selector_type>::signal_close_type
basic_client<selector_type>::close_event() const
{
	return signal_close;
}

template<typename selector_type>
typename basic_client<selector_type>::signal_login_failed_type
basic_client<selector_type>::login_failed_event() const
{
	return signal_login_failed;
}

template<typename selector_type>
typename basic_client<selector_type>::signal_login_extend_type
basic_client<selector_type>::login_extend_event() const
{
	return signal_login_extend;
}

template<typename selector_type>
void basic_client<selector_type>::on_recv_event(const packet& pack)
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

template<typename selector_type>
void basic_client<selector_type>::on_send_event()
{
	selector_type& selector = basic_object<selector_type>::get_selector();

	// Remove OUTGOING flag from selector as there is no more
	// data to send
	selector.remove(conn->get_socket(), IO_OUTGOING);
}

template<typename selector_type>
void basic_client<selector_type>::on_close_event()
{
	// Disconnect from server
	disconnect();
	// Emit close signal
	on_close();
}

template<typename selector_type>
void basic_client<selector_type>::on_join(const user& user, const packet& pack)
{
	signal_join.emit(user, pack);
}

template<typename selector_type>
void basic_client<selector_type>::on_part(const user& user, const packet& pack)
{
	signal_part.emit(user, pack);
}

template<typename selector_type>
void basic_client<selector_type>::on_data(const packet& pack)
{
	signal_data.emit(pack);
}

template<typename selector_type>
void basic_client<selector_type>::on_close()
{
	signal_close.emit();
}

template<typename selector_type>
void basic_client<selector_type>::on_login_failed(login::error error)
{
	signal_login_failed.emit(error);
}

template<typename selector_type>
void basic_client<selector_type>::on_login_extend(packet& pack)
{
	signal_login_extend.emit(pack);
}

template<typename selector_type>
void basic_client<selector_type>::net_login_failed(const packet& pack)
{
	// Received login_failed packet
	on_login_failed(
		static_cast<login::error>(
			pack.get_param(0).parameter::as<int>()
		)
	);
}

template<typename selector_type>
void basic_client<selector_type>::net_client_join(const packet& pack)
{
	// Received client_join packet
	unsigned int id = pack.get_param(0).parameter::as<int>();
	std::string name = pack.get_param(1).parameter::as<std::string>();

	user* new_client = new user(id, NULL);
	basic_object<selector_type>::user_add(new_client);
	new_client->login(name);

	// The first client who joins is the local client
	if(self == NULL) self = new_client;
	on_join(*new_client, pack);
}

template<typename selector_type>
void basic_client<selector_type>::net_client_part(const packet& pack)
{
	unsigned int id = pack.get_param(0).parameter::as<int>();
	user* rem_user = basic_object<selector_type>::user_find(id);

	if(rem_user == NULL)
		throw bad_value("Got client_part for nonexistant user");

	on_part(*rem_user, pack);
	basic_object<selector_type>::user_remove(rem_user);
}

template<typename selector_type>
void basic_client<selector_type>::connect_impl(const address& addr)
{
	// Cannot connect twice
	if(is_connected() )
		throw connected_error("net6::basic_client::connect");

	// Connect to remote host
	conn.reset(new connection(addr) );

	// Add socket to selector
	basic_object<selector_type>::get_selector().add(
		conn->get_socket(),
		IO_INCOMING | IO_ERROR
	);

	// Install signal handlers
	conn->send_event().connect(
		sigc::mem_fun(*this, &basic_client::on_send_event) );
	conn->recv_event().connect(
		sigc::mem_fun(*this, &basic_client::on_recv_event) );
	conn->close_event().connect(
		sigc::mem_fun(*this, &basic_client::on_close_event) );
}

template<typename selector_type>
void basic_client<selector_type>::
	disconnect_impl()
{
	// Not connected? Nothing to disconnect
	if(!is_connected() )
		throw not_connected_error("net6::basic_client::disconnect");

	// TODO: Remove socket from selector?

	// Reset connection, clear user list, clear self pointer
	conn.reset(NULL);
	basic_client<selector_type>::user_clear();
	self = NULL;
}

} // namespace net6

#endif // _NET6_CLIENT_HPP_
