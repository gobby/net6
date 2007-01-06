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

#ifndef _NET6_HOST_HPP_
#define _NET6_HOST_HPP_

#include <sigc++/signal.h>

#include "export.hpp"
#include "server.hpp"

namespace net6
{

/** High-level TCP host object that is used as a server with a local peer.
 */
	
class NET6_EXPORT host : public server
{
public:
	/** Participiant in a client/server network. Necessary changes
	 * to this object are performed by the net6::host object.
	 */
	typedef server::peer peer;

	/** Creates a new host object.
	 * @param username user name to use for the local peer.
	 * @param ipv6 Whether to use IPv6.
	 */
	host(const std::string& username, bool ipv6 = true);

	/** Creates a new host object which will accept incoming connections
	 * on port <em>port</em> and use the user name <em>username</em> for
	 * the local peer.
	 */
	host(unsigned int port, const std::string& username, bool ipv6 = true);
	virtual ~host();

	/** Send a packet to all the connected and logined peers.
	 */
	virtual void send(const packet& pack, peer& to);

	/** Returns the local peer
	 */
	peer* get_self() const;

protected:
	peer* self;
};
	
}

#endif

