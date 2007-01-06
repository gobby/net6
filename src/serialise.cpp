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

const char* serialise::type_name<int>::name = "int";
const char* serialise::type_name<long>::name = "long";
const char* serialise::type_name<short>::name = "short";
const char* serialise::type_name<char>::name = "char";
const char* serialise::type_name<unsigned int>::name = "unsigned int";
const char* serialise::type_name<unsigned long>::name = "unsigned long";
const char* serialise::type_name<unsigned short>::name = "unsigned short";
const char* serialise::type_name<unsigned char>::name = "unsigned char";
const char* serialise::type_name<float>::name = "float";
const char* serialise::type_name<double>::name = "double";
const char* serialise::type_name<long double>::name = "long double";
const char* serialise::type_name<bool>::name = "bool";

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

std::string serialise::default_context_to<std::string>::
	to_string(const data_type& from) const
{
	return from;
}

serialise::default_context_from<std::string>::data_type
serialise::default_context_from<std::string>::
	from_string(const data_type& from) const
{
	return from;
}

std::string serialise::default_context_to<const char*>::
	to_string(const data_type& from) const
{
	return from;
}
