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

#ifndef _NET6_OBJECT_HPP_
#define _NET6_OBJECT_HPP_

#include <map>
#include <sigc++/trackable.h>
#include "non_copyable.hpp"
#include "user.hpp"
#include "select.hpp"
#include "packet.hpp"

namespace net6
{

/** basic_object is the common base class of basic_local and basic_server
 */
template<typename selector_type>
class basic_object : public sigc::trackable, private non_copyable
{
public:
	typedef selector_type selector;

	/** Default constructor. Initializes the basic_object with an
	 * empty user list.
	 */
	basic_object();
	virtual ~basic_object();

	/** Sends a packet to the remote side(s).
	 */
	virtual void send(const packet& pack) = 0;

	/** Returns the selector used by this basic_object.
	 */
	selector_type& get_selector();

	/** Returns the selector used by this basic_object.
	 */
	const selector_type& get_selector() const;

	/** Returns the user with the given ID or NULL, if there
	 * is no such ID in the list.
	 */
	user* user_find(unsigned int id) const;

	/** Looks for a user with the given name. If there is no one, NULL
	 * is returned.
	 */
	user* user_find(const std::string& name) const;

protected:
	typedef std::map<unsigned int, user*> user_map;

	typedef typename user_map::iterator user_iterator;
	typedef typename user_map::const_iterator user_const_iterator;

	/** Internal function to add a user into the user list.
	 */
	void user_add(user* user);

	/** Internal function to remove a user from the user list.
	 */
	void user_remove(const user* user);

	/** Internal function to clear the user list.
	 */
	void user_clear();

	user_map users;
	selector_type sock_sel;
};

typedef basic_object<selector> object;

template<typename selector_type>
basic_object<selector_type>::basic_object()
{
}

template<typename selector_type>
basic_object<selector_type>::~basic_object()
{
	user_clear();
}

template<typename selector_type>
selector_type& basic_object<selector_type>::get_selector()
{
	return sock_sel;
}

template<typename selector_type>
const selector_type& basic_object<selector_type>::get_selector() const
{
	return sock_sel;

}

template<typename selector_type>
user* basic_object<selector_type>::user_find(unsigned int id) const
{
	user_const_iterator user_it = users.find(id);
	if(user_it == users.end() ) return NULL;
	return user_it->second;
}

template<typename selector_type>
user* basic_object<selector_type>::user_find(const std::string& name) const
{
	for(user_const_iterator i = users.begin(); i != users.end(); ++ i)
		if(i->second->get_name() == name)
			return i->second;

	return NULL;
}

template<typename selector_type>
void basic_object<selector_type>::user_add(user* user)
{
	users[user->get_id()] = user;
}

template<typename selector_type>
void basic_object<selector_type>::user_remove(const user* user)
{
	users.erase(user->get_id() );
	delete user;
}

template<typename selector_type>
void basic_object<selector_type>::user_clear()
{
	for(user_iterator i = users.begin(); i != users.end(); ++ i)
		delete i->second;

	users.clear();
}

} // namespace net6

#endif // _NET6_OBJECT_HPP_

