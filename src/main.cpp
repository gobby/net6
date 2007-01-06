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
#include "common.hpp"
#ifdef WIN32
#include <winsock2.h>
#endif
#include <signal.h>
#include "error.hpp"
#include "main.hpp"

unsigned int net6::main::refcount = 0;
net6::gettext_package* net6::main::package = NULL;

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
#endif

	refcount ++;

#ifdef ENABLE_NLS
	package = new gettext_package(PACKAGE, LOCALEDIR);
	init_gettext(*package);
#endif
#if 0
#ifdef ENABLE_NLS
	bindtextdomain(PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(PACKAGE, "UTF-8");
#endif
#endif
}

net6::main::~main()
{
	refcount --;
#ifdef WIN32
	if(refcount == 0)
	{
#ifdef ENABLE_NLS
		delete package;
#endif
		WSACleanup();
	}
#endif
}

