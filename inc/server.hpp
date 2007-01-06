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
	public:
		typedef connection::signal_recv_type signal_recv_type;
		typedef connection::signal_send_type signal_send_type;
		typedef connection::signal_close_type signal_close_type;

		peer(unsigned int id);
		peer(unsigned int id, const tcp_client_socket& sock,
		     const address& addr);
		~peer();

		/** Login this peer with the specified user name.
		 */
		void login(const std::string& username);

		/** Checks if this peer has been logined successfully.
		 */
		bool is_logined() const;

		/** Enqueues a packet to send it to this peer. Use
		 * net6::server::send instead, which is a wrapper around
		 * this function.
		 */
		void send(const packet& pack);

		/** Returns the size of the queue with pending packets of
		 * the connection.
		 */
		unsigned int send_queue_size() const;

		/** Returns the socket which is used to communicate with
		 * this peer.
		 */
		const tcp_client_socket& get_socket() const;

		/** Returns the remote address of this peer.
		 */
		const address& get_address() const;

		/** Signal which is emitted when data has been received from
		 * this peer.
		 */
		signal_recv_type recv_event() const;

		/** Signal which is emitted when data has been completely
		 * sent to this peer.
		 */
		signal_send_type send_event() const;

		/** Signal which is emitted when the connection to this peer
		 * has been lost.
		 */
		signal_close_type close_event() const;
	protected:
		bool logined;
		connection* conn;
	};

	/** Accumulator for signal_auth_type: It returns TRUE if no slots
	 * have been connected (so, connections are always accepted).
	 */
	class auth_accumulator
	{
	public:
		typedef bool result_type;

		template<typename iterator>
		result_type operator()(iterator begin, iterator end) const
		{
			bool result = true;
			for(; begin != end; ++ begin)
				if(!(result = *begin) )
					break;
			return result;
		}
	};

	typedef sigc::signal<void, peer&> signal_join_type;
	typedef sigc::signal<void, peer&> signal_part_type;
	typedef sigc::signal<bool, peer&, const packet&,
		std::string&>::accumulated<auth_accumulator>
			signal_login_auth_type;
	typedef sigc::signal<void, peer&, const packet&> signal_pre_login_type;
	typedef sigc::signal<void, peer&, const packet&> signal_post_login_type;
	typedef sigc::signal<void, peer&, packet&> signal_login_extend_type;
	typedef sigc::signal<void, const packet&, peer&> signal_data_type;

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

	/** Signal which is emitted when a new client joins.
	 */
	signal_join_type join_event() const;

	/** Signal which is emitted when a connection client closes the
	 * connection.
	 */
	signal_part_type part_event() const;

	/** Signal which is emitted when a client logs in with a valid user
	 * name. It is emitted before login_extend signals are emitted.
	 */
	signal_pre_login_type pre_login_event() const;

	/** Signal which is emitted when a client loggs in with a valid user
	 * name. It is emitted after login_extend signals are emitted.
	 */
	signal_post_login_type post_login_event() const;

	/** Signal used for user authentication. If the signal handler returns
	 * false, the login will be denied. The signal handler may set the
	 * third signal parameter to the reason why the login process has
	 * been denied (as human-readable string which may be sent to the
	 * client).
	 */
	signal_login_auth_type login_auth_event() const;

	/** Signal for extending the login packet sent to other clients by
	 * extra parameters.
	 */
	signal_login_extend_type login_extend_event() const;

	/** Signal which is emitted every time we received a packet from
	 * one of the connected and logged in clients
	 */
	signal_data_type data_event() const;

protected:
	virtual void remove_client(peer* client);

	virtual void on_server_read(socket& sock, socket::condition io);

	virtual void on_client_recv(const packet& pack, peer& from);
	virtual void on_client_send(const packet& pack, peer& to);
	virtual void on_client_close(peer& from);

	virtual void on_join(peer& new_peer);

	tcp_server_socket* serv_sock;
	std::list<peer*> peers;
	selector sock_sel;
	bool use_ipv6;
	unsigned int id_counter;

	signal_join_type signal_join;
	signal_part_type signal_part;
	signal_pre_login_type signal_pre_login;
	signal_post_login_type signal_post_login;
	signal_login_auth_type signal_login_auth;
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

