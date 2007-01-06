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

#ifndef _NET6_LOCAL_HPP_
#define _NET6_LOCAL_HPP_

#include "user.hpp"
#include "select.hpp"
#include "object.hpp"

namespace net6
{

/** basic_local is the common base class for all objects that have a local user
 * participating in the session, such as basic_client or basic_host.
 */
template<typename selector_type>
class basic_local: virtual public basic_object<selector_type>
{
public:
	/** Default constructor.
	 */
	basic_local();

	/** Returns the user that represents the local host.
	 */
	virtual user& get_self() = 0;

	/** Returns the user that represents the local host.
	 */
	virtual const user& get_self() const = 0;

protected:
};

typedef basic_local<selector> local;

template<typename selector_type>
basic_local<selector_type>::basic_local():
	basic_object<selector_type>()
{
}

} // namespace net6

#endif // _NET6_LOCAL_HPP_

