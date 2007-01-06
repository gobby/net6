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

#ifdef WIN32
#include <winsock2.h>
#endif

#include <signal.h>
#include "error.hpp"
#include "main.hpp"

unsigned int net6::main::refcount = 0;

net6::main::main()
{
	if(refcount > 0)
	{
		refcount ++;
		return;
	}

#ifdef WIN32
	WSAData data;
	if(WSAStartup(MAKEWORD(2, 2), &data) != 0)
		throw error(WSAGetLastError() );
#else
	signal(SIGPIPE, SIG_IGN);
#endif
	refcount ++;
}

net6::main::~main()
{
	refcount --;
#ifdef WIN32
	if(refcount == 0)
	{
		WSACleanup();
	}
#endif
}

