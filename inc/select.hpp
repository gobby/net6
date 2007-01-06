/* net6 - Library providing IPv4/IPv6 network access
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

#include "export.hpp"
#include "socket.hpp"

namespace net6
{

/** The selector may be used to wait until something occurs with a socket:
 * Either if data comes available for reading, or kernel buffer gets
 * available for writing (without blocking), or an error occures on the
 * socket.
 */
	
class NET6_EXPORT selector
{
public:
	selector();
	~selector();

	/** Adds a socket to the selector.
	 * @param sock The socket to watch for conditions to occur.
	 * @param condition Conditions the socket is watched for.
	 */
	void add(const socket& sock, socket::condition condition);

	/** Removes a socket from the selector.
	 * @param sock The socket to remove.
	 * @param condition A combination of conditions which are no
	 *        longer watched for.
	 */
	void remove(const socket& sock, socket::condition condition);

	/** Checks if a socket is watched for events
	 * @param sock Socket to check.
	 * @param condition Condition to check the socket for.
	 */
	bool check(const socket& sock, socket::condition condition);

	/** Selects infinitely until an event occurs on one or more
	 * selected sockets. Connect to the socket's signals to handle
	 * those events.
	 */
	void select();

	/** Selects until an event occurs on ore or more selectes sockets, or
	 * the timeout exceeds. Connect to the socket's signals to handle
	 * those events.
	 * @param timeout Timeout in milliseconds. May be 0 to perform
	 *        a quick poll.
	 */
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

