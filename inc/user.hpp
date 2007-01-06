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

#ifndef _NET6_USER_HPP_
#define _NET6_USER_HPP_

#include <string>
#include "non_copyable.hpp"
#include "connection.hpp"

namespace net6
{

/** Participiant in a Client/Server network.
 */

class user : private non_copyable
{
public:
	user(unsigned int unique_id, connection_base* remote_conn);

	/** Log in the user with the given name, optionally changing his ID.
	 * Note that this function is called automatically by the high-level
	 * net6 objects including client, server and host.
	 */
	void login(const std::string& user_name, unsigned int new_id = 0);

	/** Returns the unique ID for this user.
	 */
	unsigned int get_id() const;

	/** Returns whether this user is logged in.
	 */
	bool is_logged_in() const;

	/** Returns the user name of this user.
	 */
	const std::string& get_name() const;

	/** Returns the connection to the user. If there is no direct
	 * connection, not_connected_error is thrown.
	 */
	connection_base& get_connection();

	/** Returns the connection to the user. If there is no direct
	 * connection, not_connected_error is thrown.
	 */
	const connection_base& get_connection() const;

	/** Sends a packet to this user. Note that the underlaying socket will
	 * not be added to any selector. Use the send function of a high-level
	 * TCP object (such as basic_server) or add the socket of the
	 * connection to this user manually to a selector. If there is no
	 * direct connection to this user available, not_connected_error is
	 * thrown.
	 */
	void send(const packet& pack) const;

protected:
	unsigned int id;
	std::string name;
	bool logged_in;
	std::auto_ptr<connection_base> conn;
};

} // namespace net6

#endif // _NET6_USER_HPP_

