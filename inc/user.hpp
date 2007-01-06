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
	typedef sigc::signal<void> signal_encrypted_type;
	typedef sigc::signal<void> signal_encryption_failed_type;

	user(unsigned int unique_id, connection_base* remote_conn);

	/** Log in the user with the given name.
	 */
	void login(const std::string& user_name);

	/** @brief Flags the connection to this user as encrypted.
	 *
	 * This is not done automatically through the encrypted_event() of
	 * the connection since there is not a direct connection to any user.
	 */
	void set_encrypted();

	/** @brief Returns whether the connection to this user is secure.
	 */
	bool is_encrypted() const;

	/** @brief Signal that is emitted when a secure connection to this
	 * client has been established.
	 */
	signal_encrypted_type encrypted_event() const;

	/** @brief Signal that is emitted when an encryption request has
	 * been denied.
	 */
	signal_encryption_failed_type encryption_failed_event() const;

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

	/** Sends a packet to this user.
	 *
	 * If there is no direct connection to this user available,
	 * not_connected_error is thrown.
	 */
	void send(const packet& pack) const;

	/** @brief Requests an encryption connection to this client.
	 *
	 * If there is no direct connection to this user available,
	 * not_connected_error is thrown.
	 */
	void request_encryption() const;

protected:
	void on_encryption_failed();

	unsigned int id;
	std::string name;
	bool logged_in;
	std::auto_ptr<connection_base> conn;

	signal_encrypted_type signal_encrypted;
	signal_encryption_failed_type signal_encryption_failed;

	bool connection_encrypted;
};

} // namespace net6

#endif // _NET6_USER_HPP_

