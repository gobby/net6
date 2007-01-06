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

#include "export.hpp"
#include "non_copyable.hpp"
#include "socket.hpp"

namespace net6
{

/** The selector may be used to wait until something occurs with a socket:
 * Either if data comes available for reading, or kernel buffer gets
 * available for writing (without blocking), or an error occures on the
 * socket.
 */
	
class NET6_EXPORT selector : private non_copyable
{
public:
	/** Accumulator that defaults on false for signal_socket_event.
	 */
	class NET6_EXPORT socket_accumulator
	{
	public:
		typedef bool result_type;

		template<typename iterator>
		result_type operator()(iterator begin, iterator end) const
		{
			bool result = false;
			for(; begin != end; ++ begin)
				if(result = *begin)
					break;
			return result;
		}
	};

	typedef sigc::signal<bool, socket&, socket::condition>
		::accumulated<socket_accumulator> signal_socket_event_type;

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

	/** Signal which is emitted every time an event occurs on a socket.
	 * The signal handler may return true to indicate that he has handled
	 * this event. The event handler for the socket will not be
	 * emitted then. Usually you should use the socket signals to handle
	 * socket events. Use this hook only if you want to prevent the
	 * selector to emit the socket's signals. Use with care!
	 */
	signal_socket_event_type socket_event() const;

protected:
	void select_impl(timeval* tv);

	std::list<socket> read_list;
	std::list<socket> write_list;
	std::list<socket> error_list;

	signal_socket_event_type signal_socket_event;
/*
	fd_set read_set;
	fd_set write_set;
	fd_set error_set;*/
};
	
}

#endif

