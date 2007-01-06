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

net6::user::user(unsigned int unique_id, connection_base* remote_conn):
	id(unique_id), logged_in(false), conn(remote_conn),
	connection_encrypted(false)
{
	if(conn.get() != NULL)
	{
		conn->encryption_failed_event().connect(
			sigc::mem_fun(*this, &user::on_encryption_failed)
		);
	}
}

void net6::user::login(const std::string& user_name)
{
	name = user_name;
	logged_in = true;
}

void net6::user::set_encrypted()
{
	connection_encrypted = true;
	signal_encrypted.emit();
}

bool net6::user::is_encrypted() const
{
	return connection_encrypted;
}

net6::user::signal_encrypted_type net6::user::encrypted_event() const
{
	return signal_encrypted;
}

net6::user::signal_encryption_failed_type
net6::user::encryption_failed_event() const
{
	return signal_encryption_failed;
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

net6::connection_base& net6::user::get_connection()
{
	if(conn.get() == NULL)
		throw not_connected_error("net6::user::get_connection");

	return *conn.get();
}

const net6::connection_base& net6::user::get_connection() const
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

void net6::user::request_encryption() const
{
	if(conn.get() == NULL)
		throw not_connected_error("net6::user::send");

	// When we have a direct connection to a client, we are server.
	conn->request_encryption(false);
}

void net6::user::set_enable_keepalives(bool enable) const
{
	if(conn.get() == NULL)
		throw not_connected_error("net6::user::set_enable_keepalives");

	conn->set_enable_keepalives(enable);
}

void net6::user::on_encryption_failed()
{
	signal_encryption_failed.emit();
}

