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

#include "serialise.hpp"

serialise::conversion_error::conversion_error(const std::string& message):
	std::runtime_error(message)
{
}

serialise::data::data(const std::string& serialised):
	m_serialised(serialised)
{
}

const std::string& serialise::data::serialised() const
{
	return m_serialised;
}

std::string
serialise::context<std::string>::to_string(const data_type& from) const
{
	return from;
}

serialise::context<std::string>::data_type
serialise::context<std::string>::from_string(const std::string& string) const
{
	return string;
}

std::string
serialise::context<const char*>::to_string(const data_type& from) const
{
	return from;
}

