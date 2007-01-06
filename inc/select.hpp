/* net6 - library providing ipv4/ipv6 network access
 * Copyright (C) 2005 Armin Burgmeier / 0x539 dev group
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _NET6_SELECT_HPP_
#define _NET6_SELECT_HPP_

#include <net6/export.hpp>
#include <net6/socket.hpp>

namespace net6
{

class NET6_EXPORT selector
{
public:
	selector();
	~selector();

	void add(const socket& sock, socket::condition condition);
	void remove(const socket& sock, socket::condition condition);
	bool check(const socket& sock, socket::condition condition);

	void select();
	void select(unsigned long timeout);

protected:
	void select_impl(timeval* tv);

	std::list<socket> read_list;
	std::list<socket> write_list;
	std::list<socket> error_list;

	fd_set read_set;
	fd_set write_set;
	fd_set error_set;
};
	
}

#endif

