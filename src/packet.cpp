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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "packet.hpp"

const unsigned int net6::packet::DEFAULT_PRIORITY = 1000;
net6::packet::type_lookup_map net6::packet::type_map;

net6::packet::packet(unsigned int priority)
 : prio(priority)
{
}

net6::packet::packet(const std::string& command, unsigned int priority,
                     unsigned int size)
 : command(command), prio(priority)
{
	params.reserve(size);
}

net6::packet::packet(const packet& other)
 : command(other.command), prio(other.prio)
{
	params.resize(other.params.size() );
	for(std::vector<basic_parameter*>::size_type i = 0;
	    i < params.size();
	    ++i)
		params[i] = other.params[i]->clone();
}

net6::packet::~packet()
{
	for(std::vector<basic_parameter*>::const_iterator i = params.begin();
	    i != params.end();
	    ++ i)
		delete *i;
}

net6::packet& net6::packet::operator=(const packet& other)
{
	// TODO: Optimize this function by reusing existing parameters
	if(&other == this)
		return *this;

	// Delete existing parameters
	for(std::vector<basic_parameter*>::const_iterator i = params.begin();
	    i != params.end();
	    ++ i)
		delete *i;

	// Take new command
	command = other.command;

	// Take new parameters
	params.resize(other.params.size() );
	for(std::vector<basic_parameter*>::size_type i = 0;
	    i < params.size();
	    ++i)
		params[i] = other.params[i]->clone();

	return *this;
}

const std::string& net6::packet::get_command() const
{
	return command;
}

unsigned int net6::packet::get_priority() const
{
	return prio;
}

const net6::basic_parameter& net6::packet::get_param(unsigned int index) const
{
	if(index >= params.size() )
		throw basic_parameter::bad_count();

	return *params[index];
}

unsigned int net6::packet::get_param_count() const
{
	return static_cast<unsigned int>(params.size() );
}

std::string net6::packet::get_raw_string() const
{
	// TODO: Preallocate memory?
	std::string str = escape(command);
	for(std::vector<basic_parameter*>::const_iterator i = params.begin();
	    i != params.end();
	    ++ i)
	{
		const basic_parameter& param = **i;

		str += ":";
		str += param.get_type_id();
		str += escape(param.to_string() );
	}
	str += "\n";

	return str;
}

void net6::packet::set_raw_string(const std::string& raw_string)
{
	command = "";
	std::string::size_type pos = 0, prev = 0;
	while( (pos = raw_string.find(':', pos)) != std::string::npos)
	{
		set_raw_param(raw_string.substr(prev, pos - prev) );
		prev = ++ pos;
	}
	set_raw_param(raw_string.substr(prev, raw_string.length() - prev - 1) );
}

void net6::packet::register_type(identification_type type,
                                 type_lookup_slot slot)
{
	type_map[type] = slot;
}

std::string net6::packet::escape(const std::string& string)
{
	std::string escaped_string = string;
	std::string::size_type pos = 0;
	while( (pos = escaped_string.find_first_of("\\\n:", pos)) !=
				std::string::npos)
	{
		switch(escaped_string[pos])
		{
		case '\\':
			escaped_string.replace(pos, 1, "\\b");
			break;
		case '\n':
			escaped_string.replace(pos, 1, "\\n");
			break;
		case ':':
			escaped_string.replace(pos, 1, "\\d");
			break;
		}
		pos += 2;
	}
	return escaped_string;
}

std::string net6::packet::unescape(const std::string& string)
{
	std::string unescaped_string = string;
	std::string::size_type pos = 0;
	while( (pos = unescaped_string.find('\\', pos)) != std::string::npos )
	{
		if(pos < unescaped_string.length() - 1)
		{
			switch(unescaped_string[pos + 1])
			{
			case 'b':
				unescaped_string.replace(pos, 2, "\\");
				break;
			case 'n':
				unescaped_string.replace(pos, 2, "\n");
				break;
			case 'd':
				unescaped_string.replace(pos, 2, ":");
				break;
			}
		}
		++ pos;
	}
	return unescaped_string;
}

void net6::packet::set_raw_param(const std::string& param_string)
{
	// No command yet? The first parameter is the command of the packet
	if(command.empty() )
		command = unescape(param_string);
	else
	{
		type_lookup_map::const_iterator i =
			type_map.find(param_string[0]);
		// TODO: Throw error if i == type_map.end()
		params.push_back(i->second(unescape(param_string.substr(1))) );
	}
}

