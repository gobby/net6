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

#include <sigc++/bind.h>
#include "host.hpp"

net6::host::host(const std::string& username, bool ipv6)
 : server(ipv6), self(new peer(++ id_counter) )
{
	self->login(username);
	peers.push_back(self);
}

net6::host::host(unsigned int port, const std::string& username, bool ipv6)
 : server(port, ipv6), self(new peer(++ id_counter) )
{
	self->login(username);
	peers.push_back(self);
}

net6::host::~host()
{
}

void net6::host::send(const packet& pack, peer& to)
{
	if(&to != self)
		server::send(pack, to);
}

net6::host::peer* net6::host::get_self() const
{
	return self;
}

