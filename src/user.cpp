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

#include "error.hpp"
#include "user.hpp"

net6::user::user(unsigned int unique_id, connection* remote_conn)
 : id(unique_id), logged_in(false), conn(remote_conn)
{
}

void net6::user::login(const std::string& user_name, unsigned int new_id)
{
	if(logged_in)
		throw connected_error("net6::user::login");

	if(new_id != 0) id = new_id;
	name = user_name;
	logged_in = true;
}

unsigned int net6::user::get_id() const
{
	return id;
}

bool net6::user::is_logged_in() const
{
	return logged_in;
}

const std::string& net6::user::get_name() const
{
	return name;
}

net6::connection& net6::user::get_connection()
{
	if(conn.get() == NULL)
		throw not_connected_error("net6::user::get_connection");

	return *conn.get();
}

const net6::connection& net6::user::get_connection() const
{
	if(conn.get() == NULL)
		throw not_connected_error("net6::user::get_connection");

	return *conn.get();
}

void net6::user::send(const packet& pack) const
{
	if(conn.get() == NULL)
		throw not_connected_error("net6::user::send");

	conn->send(pack);
}
