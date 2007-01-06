/* net6 - Library providing IPv4/IPv6 network access
 * Copyright (C) 2005, 2006 Armin Burgmeier / 0x539 dev group
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
	typedef sigc::signal<void, const socket&, io_condition> signal_io_type;

	// Virtual destructor makes the compiler happy
	virtual ~selector() {}

	/** @brief Gets all conditions currently set on a socket
	 *
	 * The function can be used to retrieve the conditions
	 * the socket is currently watched for. It will return
	 * <em>IO_NONE</em> if the socket is not being monitored.
	 *
	 * @param sock The socket which conditions should be queried
	 */
	io_condition get(const socket& sock) const;

	/** @brief Sets conditions on a socket
	 *
	 * The function can be used to set conditions the socket is
	 * watched for. The socket will be added and deleted as
	 * deemed appropriate.
	 *
	 * @param sock The socket to watch for conditions to occur
	 * @param condition A combination of conditions to watch for. If the
	 *        condition contains net6::IO_TIMEOUT, a timeout value must
	 *        be set via the selector::set_timeout function that tells
	 *        the selector when to emit the timeout.
	 */
	void set(const socket& sock,
	         io_condition condition);

	/** @brief Emits a timeout signal for the given socket after
	 * <em>timeout</em> milliseconds.
	 *
	 * The signal is only emitted when the socket is watched for
	 * net6::IO_TIMEOUT.
	 *
	 * A previous timeout for this socket is automatically cancelled
	 * through this function. If <em>timeout</em> is 0, no new timeout
	 * is set. Note that calling set without net6::IO_TIMEOUT also cancels
	 * the timeout for a socket. The timeout is automatically unset when
	 * it has elapsed.
	 */
	void set_timeout(const socket& sock, unsigned long timeout);

	/** @brief Returns the remaining milliseconds to elapse until
	 * the timeout for the given socket is triggered.
	 *
	 * The function returns 0 if no timeout is set for the socket.
	 */
	unsigned long get_timeout(const socket& sock);

	/** @brief Selects infinitely until an event occurs on one or more
	 * selected sockets.
	 */
	void select();

	/** @brief Selects until an event occurs on ore or more
	 * selectes sockets, or the given timeout exceeds.
	 *
	 * @param timeout Timeout in milliseconds. May be 0 to perform
	 * a quick poll.
	 */
	void select(unsigned long timeout);

	/** @brief Runs in a blocking call until someone calls selector::quit.
	 */
	void run();

	/** @brief Makes the selector return from run().
	 *
	 * Does not work from a different thread as the one that called run().
	 */
	void quit();

protected:
	struct selected_type {
		io_condition condition;
		unsigned long timeout_begin;
		unsigned long timeout;
	};

	typedef std::map<const socket*, selected_type> map_type;

	void select_impl(timeval* tv);

	map_type sock_map;
	bool running;
};

}

#endif // _NET6_SELECT_HPP_
