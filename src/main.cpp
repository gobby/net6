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

#include "config.hpp"
#ifdef WIN32
#include <winsock2.h>
#endif
#ifdef ENABLE_NLS
#include <libintl.h>
#endif
#include <signal.h>
#include "error.hpp"
#include "main.hpp"
#include "packet.hpp"

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
		throw error(error::SYSTEM, WSAGetLastError() );
#else
	signal(SIGPIPE, SIG_IGN);
#endif

	// Register basic types
	packet::register_type(
		parameter<int>::TYPE_ID,
		sigc::ptr_fun(parameter<int>::from_string)
	);

	packet::register_type(
		parameter<double>::TYPE_ID,
		sigc::ptr_fun(parameter<double>::from_string)
	);

	packet::register_type(
		parameter<std::string>::TYPE_ID,
		sigc::ptr_fun(parameter<std::string>::from_string)
	);
	
	refcount ++;

#ifdef ENABLE_NLS
	bindtextdomain(PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(PACKAGE, "UTF-8");
#endif
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

