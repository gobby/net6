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

net6::selector::selector()
{
/*	FD_ZERO(&read_set);
	FD_ZERO(&write_set);
	FD_ZERO(&error_set);*/
}

net6::selector::~selector()
{
}

void net6::selector::add(const socket& sock, socket::condition condition)
{
	if(condition & socket::INCOMING)
	{
//		FD_SET(sock.cobj(), &read_set);
		read_list.push_back(sock);
	}

	if(condition & socket::OUTGOING)
	{
//		FD_SET(sock.cobj(), &write_set);
		write_list.push_back(sock);
	}

	if(condition & socket::IOERROR)
	{
//		FD_SET(sock.cobj(), &error_set);
		error_list.push_back(sock);
	}
}

void net6::selector::remove(const socket& sock, socket::condition condition)
{
	if(condition & socket::INCOMING)
	{
//		FD_CLR(sock.cobj(), &read_set);
		read_list.erase(std::remove(read_list.begin(), read_list.end(),
		                sock), read_list.end() );
	}

	if(condition & socket::OUTGOING)
	{
//		FD_CLR(sock.cobj(), &write_set);
		write_list.erase(std::remove(write_list.begin(),
		                 write_list.end(), sock), write_list.end() );
	}

	if(condition & socket::IOERROR)
	{
//		FD_CLR(sock.cobj(), &error_set);
		error_list.erase(std::remove(error_list.begin(),
		                 error_list.end(), sock), error_list.end() );
	}
}

bool net6::selector::check(const socket& sock, socket::condition condition)
{
	if(condition & socket::INCOMING)
		if(std::find(read_list.begin(), read_list.end(), sock) !=
		   read_list.end() )
			return true;

	if(condition & socket::OUTGOING)
		if(std::find(write_list.begin(), write_list.end(), sock) !=
		   write_list.end() )
			return true;

	if(condition & socket::IOERROR)
		if(std::find(error_list.begin(), error_list.end(), sock) !=
		   error_list.end() )
			return true;

	return false;
}

void net6::selector::select()
{
	select_impl(NULL);
}

void net6::selector::select(unsigned long timeout)
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

void net6::selector::select_impl(timeval* tv)
{
	// Determinate the highest file descriptor number for select()
	socket::socket_type max_fd = 0;
	fd_set readfs, writefs, errorfs;
	std::list<socket>::iterator i;

	FD_ZERO(&readfs);
	FD_ZERO(&writefs);
	FD_ZERO(&errorfs);

	for(i = read_list.begin(); i != read_list.end(); ++ i)
	{
		if(i->cobj() > max_fd)
			max_fd = i->cobj();
		FD_SET(i->cobj(), &readfs);
	}
	
	for(i = write_list.begin(); i != write_list.end(); ++ i)
	{
		if(i->cobj() > max_fd)
			max_fd = i->cobj();
		FD_SET(i->cobj(), &writefs);
	}
	
	for(i = error_list.begin(); i != error_list.end(); ++ i)
	{
		if(i->cobj() > max_fd)
			max_fd = i->cobj();
		FD_SET(i->cobj(), &errorfs);
	}

	int retval = ::select(max_fd + 1, &readfs, &writefs,
	                      &errorfs, tv);

	if(retval == -1)
		throw error();

	// Check for selected sockets
	std::list<socket> read_event, write_event, error_event;
	for(i = read_list.begin(); i != read_list.end(); ++ i)
		if(FD_ISSET(i->cobj(), &readfs) )
			read_event.push_back(*i);

	for(i = write_list.begin(); i != write_list.end(); ++ i)
		if(FD_ISSET(i->cobj(), &writefs) )
			write_event.push_back(*i);

	for(i = error_list.begin(); i != error_list.end(); ++ i)
		if(FD_ISSET(i->cobj(), &errorfs) )
			error_event.push_back(*i);

	// Reset fd_sets
/*	FD_ZERO(&read_set);
	FD_ZERO(&write_set);
	FD_ZERO(&error_set);

	for(i = read_list.begin(); i != read_list.end(); ++ i)
		FD_SET(i->cobj(), &read_set);
	for(i = write_list.begin(); i != write_list.end(); ++ i)
		FD_SET(i->cobj(), &write_set);
	for(i = error_list.begin(); i != error_list.end(); ++ i)
		FD_SET(i->cobj(), &error_set);*/

	// Emit signals in a seperate list to allow the signal
	// handlers to modify the select list
	std::list<socket>::iterator a;
	for(a = read_event.begin(); a != read_event.end(); ++ a)
		if(a->data->refcount > 1)
			if(!signal_socket_event.emit(*a, socket::INCOMING) )
				a->read_event().emit(*a, socket::INCOMING);
	for(a = write_event.begin(); a != write_event.end(); ++ a)
		if(a->data->refcount > 1)
			if(!signal_socket_event.emit(*a, socket::OUTGOING) )
				a->write_event().emit(*a, socket::OUTGOING);
	for(a = error_event.begin(); a != error_event.end(); ++ a)
		if(a->data->refcount > 1)
			if(!signal_socket_event.emit(*a, socket::IOERROR) )
				a->error_event().emit(*a, socket::IOERROR);
}
