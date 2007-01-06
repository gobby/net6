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

#include <cstring>
#include <stdexcept>

#include "queue.hpp"

net6::queue::queue():
	data(static_cast<char*>(std::malloc(1024)) ), size(0), alloc(1024),
	block_p(INVALID_POS)
{
}

net6::queue::~queue()
{
	std::free(data);
}

net6::queue::size_type net6::queue::get_size() const
{
	return block_p == INVALID_POS ? size : block_p;
}

net6::queue::size_type net6::queue::packet_size() const
{
	for(size_type i = 0; i < size; ++ i)
		if(data[i] == '\n')
			return i;

	return get_size();
}

const char* net6::queue::get_data() const
{
	return data;
}

void net6::queue::append(const char* new_data, size_type len)
{
	if(size + len > alloc)
	{
		alloc = size + len;
		data = static_cast<char*>(std::realloc(data, alloc *= 2) );
	}

	std::memcpy(data + size, new_data, len);
	size += len;
}

void net6::queue::remove(size_type len)
{
	if(len > get_size())
	{
		throw std::logic_error(
			"net6::queue::remove"
			"Cannot remove more data as there is in the queue"
		);
	}

	std::memmove(data, data + len, size - len);
	size -= len;

	if(block_p != INVALID_POS)
		block_p -= len;
}

void net6::queue::block()
{
	block_p = size;
}

void net6::queue::unblock()
{
	block_p = INVALID_POS;
}
