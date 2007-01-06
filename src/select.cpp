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

#include "error.hpp"
#include "select.hpp"

void net6::selector::add(const socket& sock,
                         io_condition condition)
{
	map_type::iterator iter = sock_map.find(&sock);

	if(iter != sock_map.end() )
	{
		iter->second |= condition;
	}
	else
	{
		sock_map[&sock] = condition;
	}
}

void net6::selector::remove(const socket& sock,
                            io_condition condition)
{
	map_type::iterator iter = sock_map.find(&sock);
	if(iter == sock_map.end() ) return;

	iter->second &= ~condition;
	if(iter->second == IO_NONE) sock_map.erase(iter);
}

void net6::selector::set(const socket& sock,
                         io_condition condition)
{
	map_type::iterator iter = sock_map.find(&sock);

	if(condition == IO_NONE && iter == sock_map.end() )
		return;
	else if(condition == IO_NONE)
		sock_map.erase(iter);
	else if(iter == sock_map.end() )
		sock_map[&sock] = condition;
	else
		iter->second = condition;
}

net6::io_condition net6::selector::check(const socket& sock,
                                         io_condition condition) const
{
	io_condition ref_cond = IO_NONE;
	map_type::const_iterator iter = sock_map.find(&sock);
	if(iter != sock_map.end() ) ref_cond = iter->second;
	return condition & ref_cond;
}

void net6::selector::select() const
{
	select_impl(NULL);
}

void net6::selector::select(unsigned long timeout) const
{
	timeval tv;
	tv.tv_sec = timeout / 1000;
	tv.tv_usec = timeout % 1000;
	select_impl(&tv);
}

net6::selector::signal_socket_event_type net6::selector::socket_event() const
{
	return signal_socket_event;
}

void net6::selector::select_impl(timeval* tv) const
{
	// Determinate the highest file descriptor number for select()
	socket::socket_type max_fd = 0;
	fd_set readfs, writefs, errorfs;

	FD_ZERO(&readfs);
	FD_ZERO(&writefs);
	FD_ZERO(&errorfs);

	for(map_type::const_iterator iter = sock_map.begin();
	    iter != sock_map.end();
	    ++ iter)
	{
		max_fd = std::max(iter->first->cobj(), max_fd);

		if(iter->second & IO_INCOMING)
			FD_SET(iter->first->cobj(), &readfs);

		if(iter->second & IO_OUTGOING)
			FD_SET(iter->first->cobj(), &writefs);

		if(iter->second & IO_ERROR)
			FD_SET(iter->first->cobj(), &errorfs);
	}

	if(::select(max_fd + 1, &readfs, &writefs, &errorfs, tv) == -1)
		throw error(net6::error::SYSTEM);

	// We pack all affected sockets into another map that is used during
	// execution of the event handlers. This allows that event handlers may
	// modify the selector's map (by performing calls to add() or remove())
	// without invalidating iterators of the select() routine here.

	map_type temp_map;
	for(map_type::const_iterator iter = sock_map.begin();
	    iter != sock_map.end();
	    ++ iter)
	{
		io_condition conds = IO_NONE;

		if(FD_ISSET(iter->first->cobj(), &readfs) )
			conds |= IO_INCOMING;
		if(FD_ISSET(iter->first->cobj(), &writefs) )
			conds |= IO_OUTGOING;
		if(FD_ISSET(iter->first->cobj(), &errorfs) )
			conds |= IO_ERROR;

		if(conds != IO_NONE)
			temp_map[iter->first] = conds;
	}

	for(map_type::const_iterator iter = temp_map.begin();
	    iter != temp_map.end();
	    ++ iter)
	{
		// Socket has been removed from the selector by the execution
		// of a previous signal handler.
		if(sock_map.find(iter->first) == sock_map.end() ) continue;

		iter->first->io_event().emit(iter->second);
	}
}

bool net6::selector::on_socket_event(const socket& sock, io_condition cond)
{
	return signal_socket_event.emit(sock, cond);
}
