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

#include <ctime>
#include <limits>
#ifdef HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif

#include "error.hpp"
#include "select.hpp"

namespace
{
	inline unsigned long time()
	{
		static std::time_t begin = 0;
		if(begin == 0) begin = std::time(NULL);

		return (std::time(NULL) - begin) * 1000;
	}

	// TODO: Subsecond time resolution...
	unsigned long msec()
	{
		static unsigned long(*time_func)() = NULL;

		if(time_func == NULL)
			time_func = &time;

		return time_func();
	}

	unsigned long time_elapsed(unsigned long since, unsigned long now)
	{
		if(since <= now) return now - since;

		throw std::logic_error(
			"net6::select.cpp::time_elapsed:\n"
			"Time overflow. Panic!"
		);
	}
}

net6::io_condition net6::selector::get(const socket& sock) const
{
	map_type::const_iterator iter = sock_map.find(&sock);

	if(iter == sock_map.end() )
		return IO_NONE;

	return iter->second.condition;
}

void net6::selector::set(const socket& sock,
                         io_condition condition)
{
	map_type::iterator iter = sock_map.find(&sock);

	if(condition != IO_NONE)
	{
		if(iter == sock_map.end() )
		{
			selected_type& type = sock_map[&sock];

			type.condition = condition;
			type.timeout_begin = 0;
			type.timeout = 0;
		}
		else
		{
			iter->second.condition = condition;

			// Cancel running timeout if IO_TIMEOUT is not set
			if( (iter->second.condition & IO_TIMEOUT) == IO_NONE)
			{
				iter->second.timeout_begin = 0;
				iter->second.timeout = 0;
			}
		}
	}
	else
	{
		// Remove socket
		if(iter != sock_map.end() )
			sock_map.erase(iter);
	}
}

void net6::selector::set_timeout(const socket& sock, unsigned long timeout)
{
	selected_type* sel_type = NULL;
	map_type::iterator iter = sock_map.find(&sock);

	if(iter != sock_map.end() )
	{
		if( (iter->second.condition & IO_TIMEOUT) == IO_TIMEOUT)
			sel_type = &iter->second;
	}

	if(sel_type == NULL)
	{
		throw std::logic_error(
			"net6::selector::set_timeout:\n"
			"Socket is not selected for IO_TIMEOUT"
		);
	}

	sel_type->timeout_begin = msec();
	sel_type->timeout = timeout;
}

unsigned long net6::selector::get_timeout(const socket& sock)
{
	map_type::const_iterator iter = sock_map.find(&sock);
	if(iter == sock_map.end() ) return 0;

	// No timeout set
	if(iter->second.timeout == 0) return 0;

	unsigned long elapsed = time_elapsed(
		iter->second.timeout_begin,
		msec()
	);

	// Timeout should already have been elapsed...
	if(elapsed >= iter->second.timeout) return 1;
	return iter->second.timeout - elapsed;
}

void net6::selector::select()
{
	select_impl(NULL);
}

void net6::selector::select(unsigned long timeout)
{
	timeval tv;
	tv.tv_sec = timeout / 1000;
	tv.tv_usec = (timeout % 1000) * 1000;
	select_impl(&tv);
}

void net6::selector::run()
{
	running = true;
	while(running)
		select();
}

void net6::selector::quit()
{
	running = false;
}

void net6::selector::select_impl(timeval* tv)
{
	socket::socket_type max_fd = 0;
	fd_set readfs, writefs, errorfs;

	unsigned long now = msec();
	unsigned long timeout = std::numeric_limits<unsigned long>::max();

	// Determinate the highest file descriptor number for select() and
	// the first timeout to be elapsed.
	FD_ZERO(&readfs);
	FD_ZERO(&writefs);
	FD_ZERO(&errorfs);

	for(map_type::const_iterator iter = sock_map.begin();
	    iter != sock_map.end();
	    ++ iter)
	{
		max_fd = std::max(iter->first->cobj(), max_fd);

		if(iter->second.condition & IO_INCOMING)
			FD_SET(iter->first->cobj(), &readfs);

		if(iter->second.condition & IO_OUTGOING)
			FD_SET(iter->first->cobj(), &writefs);

		if(iter->second.condition & IO_ERROR)
			FD_SET(iter->first->cobj(), &errorfs);

		// Can only be set if IO_TIMEOUT is set in conditions
		if( (iter->second.timeout > 0) && (timeout > 0) )
		{
			unsigned long elapsed = time_elapsed(
				iter->second.timeout_begin,
				now
			);

			// Should already have been elapsed
			if(elapsed > iter->second.timeout)
				timeout = 0;

			// Next timeout?
			if(timeout > iter->second.timeout - elapsed)
				timeout = iter->second.timeout - elapsed;
		}
	}

	// Given timeout
	if(tv != NULL)
	{
		if(timeout > (tv->tv_sec * 1000ul + tv->tv_usec / 1000ul) )
			timeout = (tv->tv_sec * 1000ul + tv->tv_usec / 1000ul);
	}

	timeval val;
	if(timeout < std::numeric_limits<unsigned long>::max() )
	{
		val.tv_sec = timeout / 1000;
		val.tv_usec = (timeout % 1000) * 1000;
		tv = &val;
	}

	if(::select(max_fd + 1, &readfs, &writefs, &errorfs, tv) == -1)
		throw error(net6::error::SYSTEM);

	now = msec();

	// We pack all affected sockets into another map that is used during
	// execution of the event handlers. This allows that event handlers may
	// modify the selector's map (by performing calls to add() or remove())
	// without invalidating iterators of the select() routine here.
	std::map<const socket*, io_condition> temp_map;
	for(map_type::iterator iter = sock_map.begin();
	    iter != sock_map.end();
	    ++ iter)
	{
		const socket* sock = iter->first;
		io_condition conds = IO_NONE;

		if(FD_ISSET(iter->first->cobj(), &readfs) )
			conds |= IO_INCOMING;
		if(FD_ISSET(iter->first->cobj(), &writefs) )
			conds |= IO_OUTGOING;
		if(FD_ISSET(iter->first->cobj(), &errorfs) )
			conds |= IO_ERROR;

		if(iter->second.timeout > 0)
		{
			if(time_elapsed(iter->second.timeout_begin, now) >=
			   iter->second.timeout)
			{
				conds |= IO_TIMEOUT;

				// Timeout has elapsed, unset
				iter->second.condition &= ~IO_TIMEOUT;
				iter->second.timeout_begin = 0;
				iter->second.timeout = 0;

				if(iter->second.condition == IO_NONE)
					sock_map.erase(iter);
			}
		}

		if(conds != IO_NONE)
			temp_map[sock] = conds;
	}

	for(std::map<const socket*, io_condition>::const_iterator iter =
		temp_map.begin();
	    iter != temp_map.end();
	    ++ iter)
	{
		// Socket has been removed from the selector by the execution
		// of a previous signal handler.
		if(sock_map.find(iter->first) == sock_map.end() ) continue;

		iter->first->io_event().emit(iter->second);
	}
}
