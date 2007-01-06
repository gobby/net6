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

#ifndef _NET6_SELECT_HPP_
#define _NET6_SELECT_HPP_

#include <map>
#include "non_copyable.hpp"
#include "default_accumulator.hpp"
#include "socket.hpp"

namespace net6
{

/** The selector may be used to wait until something occurs with a socket:
 * Either if data comes available for reading, or kernel buffer gets
 * available for writing (without blocking), or an error occurs on a
 * socket.
 */
class selector
{
public:
	typedef default_accumulator<bool, false> socket_accumulator;

	typedef sigc::signal<bool, const socket&, io_condition>
		::accumulated<socket_accumulator> signal_socket_event_type;

	// Virtual destructor makes the compiler happy
	virtual ~selector() {}

	/** @brief Adds a socket to the selector.
	 *
	 * The function may also be used to add conditions the socket
	 * is watched for.
	 *
	 * @param sock The socket to watch for conditions to occur.
	 * @param condition Conditions the socket is watched for.
	 */
	void add(const socket& sock,
	         io_condition);

	/** @brief Removes a socket from the selector.
	 *
	 * The function may also be used to remove conditions the socket
	 * is watched for. If all conditions are given, the socket is
	 * completely removed.
	 *
	 * @param sock The socket to remove.
	 * @param condition A combination of conditions which are no
	 *        longer watched for.
	 */
	void remove(const socket& sock,
	            io_condition condition);

	/** @brief Sets conditions on a socket.
	 *
	 * The function can be used to set conditions the socket is
	 * watched for. The socket will be added and deleted as
	 * deemed appropriate.
	 *
	 * @param sock The socket to watch for conditions to occur
	 * @param condition A combination of conditions to watch for
	 */
	void set(const socket& sock,
	         io_condition condition);

	/** Checks if a socket is watched for events
	 *
	 * @param sock Socket to check.
	 * @param condition Conditions to check the socket for.
	 *
	 * @return The condition the socket is currently watched for.
	 */
	io_condition check(const socket& sock,
	                   io_condition condition) const;

	/** Selects infinitely until an event occurs on one or more
	 * selected sockets. Connect to the socket's signals to handle
	 * those events.
	 */
	void select() const;

	/** Selects until an event occurs on ore or more selectes sockets, or
	 * the timeout exceeds. Connect to the socket's signals to handle
	 * those events.
	 * @param timeout Timeout in milliseconds. May be 0 to perform
	 * a quick poll.
	 */
	void select(unsigned long timeout) const;

	/** Signal which is emitted every time an event occurs on a socket.
	 * The signal handler may return true to indicate that he has handled
	 * this event. The event handler for the socket will not be
	 * emitted then. Usually you should use the socket signals to handle
	 * socket events. Use this hook only if you want to prevent the
	 * selector from emitting the socket's signals. Use with care!
	 */
	signal_socket_event_type socket_event() const;

protected:
	typedef std::map<const socket*, io_condition> map_type;

	void select_impl(timeval* tv) const;

	map_type sock_map;
	signal_socket_event_type signal_socket_event;

	/** May be overwritten to do things before/after the socket_event
	 * signal is emitted.
	 */
	virtual bool on_socket_event(const socket& sock,
	                             io_condition cond);
};
	
}

#endif

