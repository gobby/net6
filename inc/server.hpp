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

#include <sigc++/signal.h>

#include "non_copyable.hpp"
#include "default_accumulator.hpp"
#include "error.hpp"
#include "peer.hpp"
#include "address.hpp"
#include "socket.hpp"
#include "select.hpp"
#include "packet.hpp"
#include "connection.hpp"

namespace net6
{

/** High-level TCP dedicated server object.
 */

class server : public sigc::trackable, private non_copyable
{
public:
	/** Participiant in a client/server network. Necessary changes
	 * to this object are performed by the net6::server object.
	 */
	class peer : public net6::peer
	{
		friend class server;
	public:
		typedef connection::signal_send_type signal_send_type;
		typedef connection::signal_recv_type signal_recv_type;
		typedef connection::signal_close_type signal_close_type;

		peer(unsigned int id);
		peer(unsigned int id, const tcp_client_socket& sock,
		     const address& addr);
		~peer();

		/** Login this peer with the specified user name.
		 */
		void login(const std::string& username);

		/** Checks if this peer has been logged in successfully.
		 */
		bool is_logged_in() const;

		/** Returns the socket which is used to communicate with
		 * this peer.
		 */
		const tcp_client_socket& get_socket() const;

		/** Returns the remote address of this peer.
		 */
		const address& get_address() const;

		/** Signal which is emitted when data has been completely
		 * sent to this peer.
		 */
		signal_send_type send_event() const;

		/** Signal which is emitted when data has been received from
		 * this peer.
		 */
		signal_recv_type recv_event() const;

		/** Signal which is emitted when the connection to this peer
		 * has been lost.
		 */
		signal_close_type close_event() const;
	protected:
		/** Enqueues a packet to send it to this peer. Use
		 * net6::server::send instead, which is a wrapper around
		 * this function.
		 */
		void send(const packet& pack);

		bool logged_in;
		connection* conn;
	};

	typedef default_accumulator<unsigned int, 0> login_accumulator;
	typedef default_accumulator<bool, true> auth_accumulator;

	typedef sigc::signal<void, peer&> signal_connect_type;
	typedef sigc::signal<void, peer&> signal_disconnect_type;

	typedef sigc::signal<void, peer&> signal_join_type;
	typedef sigc::signal<void, peer&> signal_part_type;

	typedef sigc::signal<bool, peer&, const packet&, login::error&>
		::accumulated<auth_accumulator> signal_login_auth_type;
	typedef sigc::signal<unsigned int, peer&, const packet&>
		::accumulated<login_accumulator> signal_login_type;
	typedef sigc::signal<void, peer&, packet&> signal_login_extend_type;

	typedef sigc::signal<void, peer&, const packet&> signal_data_type;
	
	/** Creates a new server object.
	 * @param ipv6 Whether to use IPv6.
	 */
	server(bool ipv6 = true);

	/** Creates a new server which will be opened on port <em>port</em>.
	 */
	server(unsigned int port, bool ipv6 = true);
	virtual ~server();

	/** Shuts down the server socket. New connections will no longer be
	 * accepted, but already established connections stay open.
	 */
	virtual void shutdown();

	/** (re)opens the server socket on port <em>port</em>, if it has
	 * been shut down before.
	 */
	virtual void reopen(unsigned int port);

	/** Removes the connection to the given peer.
	 */
	void kick(peer& client);

	/** Wait infinitely for incoming events. The events themselves
	 * are handled by the server.
	 */
	virtual void select();

	/** Wait for incoming events or until <em>timeout</em> exceeds. The
	 * events themselves are handled by the server.
	 */
	virtual void select(unsigned int timeout);

	/** Send a packet to all the connected and logined peers.
	 */
	void send(const packet& pack);

	/** Send a packet to a single peer.
	 */
	virtual void send(const packet& pack, peer& to);

	/** Lookup a peer with the given id. If the peer is not connected to
	 * the server, NULL is returned.
	 */
	peer* find(unsigned int id) const;

	/** Look for a peer whose user name is <em>name</em>. If no one is
	 * found, NULL is returned.
	 */
	peer* find(const std::string& name) const;

	/** Returns the underlaying TCP server socket object. Note that this
	 * function will cause a segmentation fault if the server has not
	 * been opened.
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
	 * define any. The signal hanlder may return an ID for the new client.
	 * Be sure that the given ID is not alreayd in use. If it returns the
	 * special value 0, net6 chooses automatically a new id.
	 */
	signal_login_type login_event() const;

	/** Signal which may be used to append parameters to a client_join
	 * packet which will be sent to existing users to announce the new join.
	 * The first parameter is the new client's ID number, the second one
	 * its user name, other parameters may be appended by you. The peer
	 * given to the signal handler is the peer for which information has
	 * to be appended, not the one to which they will be sent.
	 */
	signal_login_extend_type login_extend_event() const;

	/** Signal which will be emitted when a packet from a client has
	 * arrived.
	 */
	signal_data_type data_event() const;
	
protected:
	virtual void remove_client(peer* client);

	virtual void on_accept_event(socket::condition io);
	virtual void on_send_event(peer& to);
	virtual void on_recv_event(const packet& pack, peer& from);
	virtual void on_close_event(peer& from);

	virtual void on_connect(peer& client);
	virtual void on_disconnect(peer& client);
	virtual void on_join(peer& client);
	virtual void on_part(peer& client);
	virtual bool on_login_auth(peer& client, const packet& pack,
	                           login::error& error);
	virtual unsigned int on_login(peer& client, const packet& pack);
	virtual void on_login_extend(peer& client, packet& pack);
	virtual void on_data(peer& client, const packet& pack);

	virtual void net_client_login(peer& from, const packet& pack);

	tcp_server_socket* serv_sock;
	std::list<peer*> peers;
	selector sock_sel;
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
	/** Private implementations for the shutdown() and reopen() functions.
	 * They are used by shutdown() and reopen() as well as by the
	 * constructor/destructor. They are necessary because a normal
	 * reopen()-Call in the constructor is not treated virtual because a
	 * derived object is not instanciated at this point. The code in these
	 * functions can not be executed directly by reopen() or shutdown()
	 * because those are virtual functions, so they would be called twice
	 * in a derived object (net6::server::server would call
	 * net6::server::reopen, and derived::derived would call derived::reopen
	 * which would call its base function, net6::server::reopen to provide
	 * its functionallity in the case that it is not called by a constructor
	 */
	void shutdown_impl();
	void reopen_impl(unsigned int port);
};
	
}

#endif

