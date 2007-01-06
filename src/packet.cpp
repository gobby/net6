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
#include "connection.hpp"

net6::parameter::parameter(const std::string& value):
	m_value(value)
{
}

const std::string& net6::parameter::serialised() const
{
	return m_value.serialised();
}

net6::packet::packet(const std::string& command,
                     unsigned int size):
	command(command)
{
	params.reserve(size);
}

net6::packet::packet(connection::queue& queue)
{
	// Check for a complete packet on the queue
	connection::queue::size_type pack_pos = queue.packet_size();
	if(pack_pos == queue.get_size() )
		throw end_of_queue();

	// Get packet string representation
	std::string pack_string(queue.get_data(), pack_pos);
	queue.remove(pack_pos + 1);

	// Find command
	std::string::size_type pos = pack_string.find(':');
	if(pos == std::string::npos) pos = pack_string.length();
	command = unescape(pack_string.substr(0, pos) );

	// Parameters
	std::string::size_type prev = ++ pos;
	while( (pos = pack_string.find(':', pos)) != std::string::npos)
	{
		std::string::size_type len = pos - prev;
		params.push_back(
			parameter(unescape(pack_string.substr(prev, len)))
		);

		prev = ++ pos;
	}

	// Last one
	if(prev <= pack_string.length() )
		params.push_back(parameter(unescape(pack_string.substr(prev))));
}

const std::string& net6::packet::get_command() const
{
	return command;
}

const net6::parameter& net6::packet::get_param(unsigned int index) const
{
	if(index >= params.size() )
		throw bad_count();

	return params[index];
}

unsigned int net6::packet::get_param_count() const
{
	return static_cast<unsigned int>(params.size() );
}

void net6::packet::enqueue(connection::queue& queue) const
{
	// Packet command
	std::string escaped_command = escape(command);
	queue.append(escaped_command.c_str(), escaped_command.length() );

	for(std::vector<parameter>::const_iterator iter = params.begin();
	    iter != params.end();
	    ++ iter)
	{
		// Parameter separator
		queue.append(":", 1);

		// Next parameter
		std::string escaped_param = escape(iter->serialised() );
		queue.append(escaped_param.c_str(), escaped_param.length() );
	}

	// Packet separator
	queue.append("\n", 1);
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

