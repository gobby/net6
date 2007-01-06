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

#include <iostream>
#include "non_copyable.hpp"

net6::non_copyable::non_copyable()
{
}

net6::non_copyable::~non_copyable()
{
}

net6::non_copyable::non_copyable(const non_copyable& other)
{
	std::cerr << "Warning: Copying non_copyable" << std::endl;
}

net6::non_copyable& net6::non_copyable::operator=(const non_copyable& other)
{
	std::cerr << "Warning: Assigning non_copyable" << std::endl;
	return *this;
}

