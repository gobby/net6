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

#ifndef _NET6_SERVER_HPP_
#define _NET6_SERVER_HPP_

#include <memory>
#include <sigc++/signal.h>
#include <sigc++/bind.h>

#include "non_copyable.hpp"
#include "default_accumulator.hpp"
#include "error.hpp"
#include "user.hpp"
#include "address.hpp"
#include "socket.hpp"
#include "select.hpp"
#include "packet.hpp"
#include "connection.hpp"
#include "object.hpp"

namespace net6
{

/** High-level TCP dedicated server object.
 */
template<typename selector_type>
class basic_server : virtual public basic_object<selector_type>
{
public:
	typedef connection<selector_type> connection_type;

	typedef default_accumulator<bool, true> auth_accumulator;

	typedef sigc::signal<void, const user&>
		signal_connect_type;
	typedef sigc::signal<void, const user&>
		signal_disconnect_type;

	typedef sigc::signal<void, const user&>
		signal_join_type;
	typedef sigc::signal<void, const user&>
		signal_part_type;

	typedef typename
		sigc::signal<bool, const user&, const packet&, login::error&>::
		template accumulated<auth_accumulator> signal_login_auth_type;
	typedef sigc::signal<void, const user&, const packet&>
		signal_login_type;

	typedef sigc::signal<void, const user&, packet&>
		signal_login_extend_type;

	typedef sigc::signal<void, const user&, const packet&>
		signal_data_type;

	/** Creates a new basic_server object.
	 * @param ipv6 Whether to use IPv6.
	 */
	basic_server(bool ipv6 = true);

	/** Creates a new basic_server which will be opened on port
	 * <em>port</em>.
	 */
	basic_server(unsigned int port, bool ipv6 = true);
	virtual ~basic_server();

	/** (re)opens the server socket on port <em>port</em>, if it has
	 * been shut down before.
	 */
	virtual void reopen(unsigned int port);

	/** Shuts down the server socket. New connections will no longer be
	 * accepted, but already established connections stay open.
	 */
	virtual void shutdown();

	/** Returns whether the server socket has been opened. Note that the
	 * socket may not be open but there are still client connections if the
	 * server has been shut down when clients were connected.
	 */
	bool is_open() const;

	/** Removes the connection to the given user.
	 */
	void kick(const user& user);

	/** Send a packet to all the connected and logged in users.
	 */
	virtual void send(const packet& pack);

	/** Send a packet to a single user.
	 */
	virtual void send(const packet& pack, const user& to);

	/** @brief Requests secure communication with the given user.
	 */
	virtual void request_encryption(const user& to);

	/** Returns the underlaying TCP server socket object. The function
	 * throws not_connected_error if the server has not been opened.
	 */
	const tcp_server_socket& get_socket() const;

	/** Signal which is emitted when a new connection has been accepted.
	 * The signal handler may return an ID for the new client. Be sure that
	 * the ID is not already in use. If the signal handler returns the
	 * special value 0, net6 chooses automatically a new ID.
	 */
	signal_connect_type connect_event() const;

	/** Signal which is emitted when a connection has been lost.
	 */
	signal_disconnect_type disconnect_event() const;
	
	/** Signal which is emitted when a new client joins the net6 session,
	 * that means, that he logged in successfully and the login procedure
	 * has finished. This is a good place to send any other initial data
	 * to the new client.
	 */
	signal_join_type join_event() const;

	/** Signal which is emitted when a client quits the session. Normally,
	 * this is called when the user has lost its connection (a disconnect
	 * event will follow), so do better not send anything to the client
	 * which has quit.
	 */
	signal_part_type part_event() const;

	/** Signal which may be used to prevent that a user joins the session.
	 * Returning false means that the login has failed, the login::error
	 * parameter may be used to describe why the login failed. You can
	 * declare your own errors and assign them to this variable. These
	 * should have values between net6::login::ERROR_MAX + 1 to UINT32_MAX.
	 * This variable will be sent with the login_failed packet. The client
	 * may then show up an error string reporting what has gone wrong.
	 */
	signal_login_auth_type login_auth_event() const;

	/** Signal which is emitted when a client loggs in with a valid
	 * user name and if signal_login_auth returned true. This is a good
	 * place to put the client into a list or something, so that the
	 * login_extend signal handler finds the new client. Do not send any
	 * packets to this client unless you know what you are doing: They
	 * will be sent before the net6 user list synchronisation! The first
	 * parameter in the login packet is always the user name the client
	 * would like to have. Others are set by the client's login_extend
	 * signal handler. Check in login_auth if they are correct, if you
	 * define any.
	 */
	signal_login_type login_event() const;

	/** Signal which may be used to append parameters to a client_join
	 * packet which will be sent to existing users to announce the new join.
	 * The first parameter is the new client's ID number, the second one
	 * its user name, the third its encryption state. Other parameters may
	 * be appended by you. The user given to the signal handler is the use
	 * for which information has to be appended, not the one to which they
	 * will be sent.
	 */
	signal_login_extend_type login_extend_event() const;

	/** Signal which will be emitted when a packet from a client has
	 * arrived.
	 */
	signal_data_type data_event() const;
	
protected:
	void remove_client(const user* client);

	void on_accept_event(io_condition io);
	void on_recv_event(const packet& pack,
	                   user& from);
	void on_close_event(user& user);
	void on_encrypted_event(user& user);

	virtual void on_connect(const user& user);
	virtual void on_disconnect(const user& user);
	virtual void on_join(const user& user);
	virtual void on_part(const user& user);
	virtual bool on_login_auth(const user& user,
	                           const packet& pack,
	                           login::error& error);
	virtual void on_login(const user& user,
	                      const packet& pack);
	virtual void on_login_extend(const user& user,
	                             packet& pack);
	virtual void on_data(const user& user,
	                     const packet& pack);

	virtual void net_client_login(user& from, const packet& pack);

	std::auto_ptr<tcp_server_socket> serv_sock;
	bool use_ipv6;
	unsigned int id_counter;

	signal_connect_type signal_connect;
	signal_disconnect_type signal_disconnect;
	signal_join_type signal_join;
	signal_part_type signal_part;
	signal_login_auth_type signal_login_auth;
	signal_login_type signal_login;
	signal_login_extend_type signal_login_extend;
	signal_data_type signal_data;
	
private:
	void shutdown_impl();
	void reopen_impl(unsigned int port);
};

typedef basic_server<selector> server;

template<typename selector_type>
basic_server<selector_type>::basic_server(bool ipv6)
 : use_ipv6(ipv6), id_counter(0)
{
}

template<typename selector_type>
basic_server<selector_type>::basic_server(unsigned int port, bool ipv6)
 : use_ipv6(ipv6), id_counter(0)
{
	reopen_impl(port);
}

template<typename selector_type>
basic_server<selector_type>::~basic_server()
{
	// TODO: Call user_clear first to remove user connections first?
	if(is_open() )
		shutdown_impl();
}

template<typename selector_type>
void basic_server<selector_type>::reopen(unsigned int port)
{
	reopen_impl(port);
}

template<typename selector_type>
void basic_server<selector_type>::shutdown()
{
	shutdown_impl();
}

template<typename selector_type>
bool basic_server<selector_type>::is_open() const
{
	return serv_sock.get() != NULL;
}

template<typename selector_type>
void basic_server<selector_type>::kick(const user& user)
{
	remove_client(&user);
}

template<typename selector_type>
void basic_server<selector_type>::send(const packet& pack)
{
	for(typename basic_object<selector_type>::user_iterator i =
		basic_object<selector_type>::users.begin();
	    i != basic_object<selector_type>::users.end();
	    ++ i)
	{
		if(i->second->is_logged_in() )
			send(pack, *i->second);
	}
}

template<typename selector_type>
void basic_server<selector_type>::send(const packet& pack, const user& to)
{
	// Enqueue packet
	to.send(pack);
}

template<typename selector_type>
void basic_server<selector_type>::request_encryption(const user& to)
{
	to.request_encryption();
}

template<typename selector_type>
const tcp_server_socket& basic_server<selector_type>::get_socket() const
{
	if(!is_open() )
		throw not_connected_error("net6::basic_server::get_socket");

	return *serv_sock;
}

template<typename selector_type>
typename basic_server<selector_type>::signal_connect_type
basic_server<selector_type>::connect_event() const
{
	return signal_connect;
}

template<typename selector_type>
typename basic_server<selector_type>::signal_disconnect_type
basic_server<selector_type>::disconnect_event() const
{
	return signal_disconnect;
}

template<typename selector_type>
typename basic_server<selector_type>::signal_join_type
basic_server<selector_type>::join_event() const
{
	return signal_join;
}

template<typename selector_type>
typename basic_server<selector_type>::signal_part_type
basic_server<selector_type>::part_event() const
{
	return signal_part;
}

template<typename selector_type>
typename basic_server<selector_type>::signal_login_auth_type
basic_server<selector_type>::login_auth_event() const
{
	return signal_login_auth;
}

template<typename selector_type>
typename basic_server<selector_type>::signal_login_type
basic_server<selector_type>::login_event() const
{
	return signal_login;
}

template<typename selector_type>
typename basic_server<selector_type>::signal_login_extend_type
basic_server<selector_type>::login_extend_event() const
{
	return signal_login_extend;
}

template<typename selector_type>
typename basic_server<selector_type>::signal_data_type
basic_server<selector_type>::data_event() const
{
	return signal_data;
}

template<typename selector_type>
void basic_server<selector_type>::remove_client(const user* user)
{
	// Emit part/disconnect signals
	if(user->is_logged_in() )
		on_part(*user);
	on_disconnect(*user);

	// Store ID of client to remove
	unsigned int user_id = user->get_id();
	// Remove user to prevent server from sending the packet to the
	// user we are currently removing
	basic_object<selector_type>::user_remove(user);

	// Build packet for other clients
	if(user->is_logged_in() )
	{
		packet pack("net6_client_part");
		pack << user->get_id();
		send(pack);
	}
}

template<typename selector_type>
void basic_server<selector_type>::on_accept_event(io_condition io)
{
	// Get selector from base class
	selector_type& selector = basic_object<selector_type>::get_selector();
	connection_type* conn = new connection_type(selector);
	std::auto_ptr<user> client(new user(++ id_counter, conn) );

	conn->recv_event().connect(
		sigc::bind(
			sigc::mem_fun(*this, &basic_server::on_recv_event),
			sigc::ref(*client)
		)
	);

	conn->close_event().connect(
		sigc::bind(
			sigc::mem_fun(*this, &basic_server::on_close_event),
			sigc::ref(*client)
		)
	);

	conn->encrypted_event().connect(
		sigc::bind(
			sigc::mem_fun(*this, &basic_server::on_encrypted_event),
			sigc::ref(*client)
		)
	);

	// Accept new client connection
	if(use_ipv6)
	{
		ipv6_address addr;

		std::auto_ptr<tcp_client_socket> new_sock(
			serv_sock->accept(addr)
		);

		conn->assign(new_sock, addr);
	}
	else
	{
		ipv4_address addr;

		std::auto_ptr<tcp_client_socket> new_sock(
			serv_sock->accept(addr)
		);

		conn->assign(new_sock, addr);
	}

	basic_object<selector_type>::user_add(client.get() );

	// Emit connection signal for new client
	on_connect(*client.release() );
}

template<typename selector_type>
void basic_server<selector_type>::on_recv_event(const packet& pack, user& from)
{
	if(pack.get_command() == "net6_client_login")
		net_client_login(from, pack);
	else
		if(from.is_logged_in() )
			on_data(from, pack);
}

template<typename selector_type>
void basic_server<selector_type>::on_close_event(user& user)
{
	remove_client(&user);
}

template<typename selector_type>
void basic_server<selector_type>::on_encrypted_event(user& user)
{
	user.set_encrypted();

	// Tell about encrypted connection
	net6::packet encr_pack("net6_encryption_info");
	encr_pack << user.get_id();
	send(encr_pack);
}

template<typename selector_type>
void basic_server<selector_type>::on_connect(const user& user)
{
	signal_connect.emit(user);
}

template<typename selector_type>
void basic_server<selector_type>::on_disconnect(const user& user)
{
	signal_disconnect.emit(user);
}

template<typename selector_type>
void basic_server<selector_type>::on_join(const user& user)
{
	signal_join.emit(user);
}

template<typename selector_type>
void basic_server<selector_type>::on_part(const user& user)
{
	signal_part.emit(user);
}

template<typename selector_type>
bool basic_server<selector_type>::
	on_login_auth(const user& user, const packet& pack, login::error& error)
{
	return signal_login_auth.emit(user, pack, error);
}

template<typename selector_type>
void basic_server<selector_type>::
	on_login(const user& user, const packet& pack)
{
	signal_login.emit(user, pack);
}

template<typename selector_type>
void basic_server<selector_type>::on_login_extend(const user& user, packet& pack)
{
	signal_login_extend.emit(user, pack);
}

template<typename selector_type>
void basic_server<selector_type>::on_data(const user& user, const packet& pack)
{
	signal_data.emit(user, pack);
}

template<typename selector_type>
void basic_server<selector_type>::net_client_login(user& user, const packet& pack)
{
	// Is already logged in
	if(user.is_logged_in() ) return;

	// Get wished user name
	// TODO: trim name?
	const std::string& name =
		pack.get_param(0).parameter::as<std::string>();

	// Check for valid user name
	if(name.empty() )
	{
		packet pack("net6_login_failed");
		pack << static_cast<int>(login::ERROR_NAME_INVALID);
		send(pack, user);
	}
	// Check for existing user name
	else if(basic_object<selector_type>::user_find(name) != NULL)
	{
		packet pack("net6_login_failed");
		pack << static_cast<int>(login::ERROR_NAME_IN_USE);
		send(pack, user);
	}
	else
	{
		// Check for login_auth
		login::error reason;
		if(!on_login_auth(user, pack, reason) )
		{
			packet pack("net6_login_failed");
			pack << static_cast<int>(reason);
			send(pack, user);
			return;
		}

		// Login succeeded
		user.login(name);
		on_login(user, pack);

		// Synchronise with other clients
		packet self_pack("net6_client_join");
		self_pack << user.get_id() << name << user.is_encrypted();
		on_login_extend(user, self_pack);
		send(self_pack, user);

		for(typename basic_object<selector_type>::user_const_iterator
			iter = basic_object<selector_type>::users.begin();
		    iter != basic_object<selector_type>::users.end();
		    ++ iter)
		{
			if(!iter->second->is_logged_in() ) continue;
			if(iter->second == &user) continue;

			packet join_pack("net6_client_join");
			join_pack << iter->second->get_id()
			          << iter->second->get_name()
			          << iter->second->is_encrypted();
			on_login_extend(*iter->second, join_pack);

			send(join_pack, user);
			send(self_pack, *iter->second);
		}

		// Join complete
		on_join(user);
	}
}

template<typename selector_type>
void basic_server<selector_type>::shutdown_impl()
{
	selector_type& selector = basic_object<selector_type>::get_selector();

	selector.set(*serv_sock, selector.get(*serv_sock) & ~IO_INCOMING);
	serv_sock.reset(NULL);
}

template<typename selector_type>
void basic_server<selector_type>::reopen_impl(unsigned int port)
{
	// Open socket on local port
	if(use_ipv6)
	{
		ipv6_address bind_addr(port);
		serv_sock.reset(new tcp_server_socket(bind_addr) );
	}
	else
	{
		ipv4_address bind_addr(port);
		serv_sock.reset(new tcp_server_socket(bind_addr) );
	}

	selector_type& selector = basic_object<selector_type>::get_selector();

	// Add incoming flag to selector to accept client connections
	selector.set(*serv_sock,
		selector.get(*serv_sock) | IO_INCOMING);
	serv_sock->io_event().connect(
		sigc::mem_fun(*this, &basic_server::on_accept_event) );
}

} // namespace net6

#endif // _NET6_SERVER_HPP_

