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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <sstream>
#include "packet.hpp"

namespace
{
	template<typename to_type, typename from_type>
	to_type convert(const from_type& from)
	{
		std::stringstream conv_stream;
		conv_stream << from;
		to_type to;
		conv_stream >> to;
		return to;
	}

	template<typename to_type>
	to_type from_string(const std::string& string)
	{
		return convert<to_type, std::string>(string);
	}

	template<typename from_type>
	std::string to_string(const from_type& from)
	{
		return convert<std::string, from_type>(from);
	}
}

net6::packet::param::param()
 : type(INT)
{
}

net6::packet::param::param(int val)
 : type(INT)
{
	data.i = val;
}

net6::packet::param::param(float val)
 : type(FLOAT)
{
	data.f = val;
}

net6::packet::param::param(const std::string& val)
 : type(STRING)
{
	data.s = new std::string(val);
}

net6::packet::param::param(const param& other)
 : type(other.type)
{
	switch(type)
	{
	case INT:
		data.i = other.data.i;
		break;
	case FLOAT:
		data.f = other.data.f;
		break;
	case STRING:
		data.s = new std::string(*other.data.s);
		break;
	}
}

net6::packet::param::~param()
{
	clear_memory();
}

net6::packet::param& net6::packet::param::operator=(const param& other)
{
	if(&other == this)
		return *this;

	switch(other.type)
	{
	case INT:
		clear_memory();
		data.i = other.data.i;
		break;
	case FLOAT:
		clear_memory();
		data.f = other.data.f;
		break;
	case STRING:
		if(type == STRING)
			*data.s = *other.data.s;
		else
			data.s = new std::string(*other.data.s);
	}

	type = other.type;
	return *this;
}

int net6::packet::param::as_int() const
{
	return data.i;
}

float net6::packet::param::as_float() const
{
	return data.f;
}

const std::string& net6::packet::param::as_string() const
{
	return *data.s;
}

net6::packet::param::type_type net6::packet::param::get_type() const
{
	return type;
}

void net6::packet::param::clear_memory()
{
	if(type == STRING)
		delete data.s;
}

net6::packet::packet()
{
}

net6::packet::packet(const std::string& command, unsigned int size)
 : command(command)
{
	params.reserve(size);
}

net6::packet::packet(const packet& other)
 : command(other.command)
{
	params.resize(other.params.size() );
	std::copy(other.params.begin(), other.params.end(), params.begin() );
}

net6::packet::~packet()
{
}

net6::packet& net6::packet::operator=(const packet& other)
{
	if(&other == this)
		return *this;

	command = other.command;

	params.resize(other.params.size() );
	std::copy(other.params.begin(), other.params.end(), params.begin() );

	return *this;
}

const std::string& net6::packet::get_command() const
{
	return command;
}

const net6::packet::param& net6::packet::get_param(unsigned int index) const
{
	return params[index];
}

unsigned int net6::packet::get_param_count() const
{
	return static_cast<unsigned int>(params.size() );
}

std::string net6::packet::get_raw_string() const
{
	std::vector<param>::const_iterator i;

	// Calculate string size
	std::string::size_type str_size = command.length() + 2;
	for(i = params.begin(); i != params.end(); ++ i)
	{
		switch(i->get_type() )
		{
		case param::INT:
			str_size += 12;
			break;
		case param::FLOAT:
			str_size += 12;
			break;
		case param::STRING:
			str_size += (i->as_string().length() + 2);
			break;
		}
	}

	std::string str;
	str.reserve(str_size);
	str = escape(command);

	for(i = params.begin(); i != params.end(); ++ i)
	{
		switch(i->get_type() )
		{
		// First parameter character indicates parameter type
		case param::INT:
			str += (":i" + to_string<int>(i->as_int()) );
			break;
		case param::FLOAT:
			str += (":f" + to_string<float>(i->as_float()) );
			break;
		case param::STRING:
			str += (":s" + escape(i->as_string()) );
			break;
		}
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
		// Check parameter type
		switch(param_string[0])
		{
		case 'i':
			params.push_back(param(
				from_string<int>(param_string.substr(1) ) ));
			break;
		case 'f':
			params.push_back(param(
				from_string<float>(param_string.substr(1) ) ));
			break;
		case 's':
			params.push_back(param(
				unescape(param_string.substr(1)) ) );
			break;
		}
	}
}

