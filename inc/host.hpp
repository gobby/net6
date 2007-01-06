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

#ifndef _NET6_HOST_HPP_
#define _NET6_HOST_HPP_

#include "local.hpp"
#include "server.hpp"

namespace net6
{

/** High-level TCP host object that is used as a server with a local user.
 */

template<typename selector_type = net6::selector>
class basic_host
 : virtual public basic_local<selector_type>,
   virtual public basic_server<selector_type>
{
public:
	/** Creates a new basic_host object.
	 * @param username user name to use for the local user.
	 * @param ipv6 Whether to use IPv6.
	 */
	basic_host(const std::string& username, bool ipv6 = true);

	/** Creates a new basic_host object which will accept incoming
	 * connections on port <em>port</em> and use the user name
	 * <em>username</em> for the local user.
	 */
	basic_host(unsigned int port, const std::string& username,
	           bool ipv6 = true);

	/** Sends a packet to a single user. The request is ignored if
	 * <em>to</em> is the local user.
	 */
	virtual void send(const packet& pack, const user& to);

	/** @brief Requests encryption to the given user.
	 *
	 * Throws an error if <em>to</em> is the local user.
	 */
	virtual void request_encryption(const user& to);

	/** Returns the local user.
	 */
	virtual user& get_self();

	/** Returns the local user.
	 */
	virtual const user& get_self() const;

protected:
	user* self;
};

typedef basic_host<selector> host;

template<typename selector_type>
basic_host<selector_type>::basic_host(const std::string& username, bool ipv6)
 : basic_object<selector_type>(),
   basic_local<selector_type>(),
   basic_server<selector_type>(ipv6),
   self(new user(++ basic_server<selector_type>::id_counter, NULL) )
{
	self->login(username);
	basic_object<selector_type>::user_add(self);
}

template<typename selector_type>
basic_host<selector_type>::
	basic_host(unsigned int port, const std::string& username, bool ipv6)
 : basic_object<selector_type>(),
   basic_local<selector_type>(),
   basic_server<selector_type>(port, ipv6),
   self(new user(++ basic_server<selector_type>::id_counter, NULL) )
{
	self->login(username);
	basic_object<selector_type>::user_add(self);
}

template<typename selector_type>
void basic_host<selector_type>::send(const packet& pack, const user& to)
{
	if(&to != self) basic_server<selector_type>::send(pack, to);
}

template<typename selector_type>
void basic_host<selector_type>::request_encryption(const user& to)
{
	if(&to == self)
	{
		throw std::logic_error(
			"net6::basic_host::request_encryption:\n"
			"Cannot request encryption to local client"
		);
	}

	basic_server<selector_type>::request_encryption(to);
}

template<typename selector_type>
user& basic_host<selector_type>::get_self()
{
	return *self;
}

template<typename selector_type>
const user& basic_host<selector_type>::get_self() const
{
	return *self;
}

} // namespace net6

#endif // _NET6_HOST_HPP_

