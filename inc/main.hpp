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

#ifndef _NET6_MAIN_HPP_
#define _NET6_MAIN_HPP_

#include "non_copyable.hpp"

/** The main net6 namespace where all net6 stuff belongs to.
 */
namespace net6
{

/** net6 main object. Every net6 capable program needs one of these objects
 * to (de)initialise the library.
 */
	
class main : private non_copyable
{
public:
	main();
	~main();

private:
	static unsigned int refcount;
};

}

#endif
