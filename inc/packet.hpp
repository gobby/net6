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

#ifndef _NET6_PACKET_HPP_
#define _NET6_PACKET_HPP_

#include <string>
#include <vector>
#include "export.hpp"

namespace net6
{

/** Represents a packet to send through the network to another host. A packet
 * exists of a command and zero ore more parameters of type int, float, or
 * std::string.
 */
	
class NET6_EXPORT packet
{
public:

	/** Parameter for a packet. It may be of type int, float or std::string.
	 */
	
	class NET6_EXPORT param
	{
	public:
		/** Available types that a packet parameter may have
		 */
		enum type_type {
			INT,
			FLOAT,
			STRING
		};

		/** Creates an empty parameter with an uninitialized value.
		 */
		param();

		/** Creates a new integer parameter.
		 */
		param(int val);

		/** Creates a new unsigned integer parameter. Note that this
		 * constructor simply stores the unsigned int value in int and
		 * may be retrieved by called as_int(). It is used to use
		 * packet::operator<< with unsigned int as parameter.
		 */
		param(unsigned int val);

		/** Creates a new float parameter.
		 */
		param(float val);

		/** Creates a new string parameter.
		 */
		param(const std::string& val);

		/** Creates a copy of another parameter. It will have the same
		 * type and value as <em>other</em>.
		 */
		param(const param& other);
		~param();

		/** Copies type and data of <em>other</em>
		 */
		param& operator=(const param& other);

		/** Returns the integer value of the parameter. The parameter
		 * has to be of type int.
		 */
		int as_int() const;

		/** Returns the float value of the parameter. The parameter has
		 * to be of type float.
		 */
		float as_float() const;

		/** Returns the string value of the parameter. The parameter
		 * has to be of type string.
		 */
		const std::string& as_string() const;

		/** Returns the type of this parameter.
		 */
		type_type get_type() const;
	protected:
		void clear_memory();

		union data_type
		{
			int i;
			float f;
			std::string* s;
		};
			
		type_type type;
		data_type data;
	};

	/** Default priority for packets.
	 */
	static const unsigned int DEFAULT_PRIORITY;

	/** Creates an empty packet.
	 */
	packet(unsigned int priority = DEFAULT_PRIORITY);

	/** Creates a new packet.
	 * @param command Command for the packet.
	 * @param priority Packet priority. Packets with higher priority are
	 * sent before packets with lower priority.
	 * @param size Number of parameters the packet will preallocate memory
	 * for.
	 */
	packet(const std::string& command,
	       unsigned int priority = DEFAULT_PRIORITY,
	       unsigned int size = 0);

	/** Creates a copy of <em>other</em>.
	 */
	packet(const packet& other);
	~packet();

	/** Creates a copy of <em>other</em>.
	 */
	packet& operator=(const packet& other);

	/** Adds a new parameter to this packet.
	 */
	template<typename T> packet& operator<<(const T& val)
	{
		params.push_back(param(val) );
		return *this;
	}

	/** Returns the command of this packet.
	 */
	const std::string& get_command() const;

	/** Returns the priority of this packet. Packets with higher priority
	 * are sent before ones with lower priority.
	 */
	unsigned int get_priority() const;

	/** Returns the <em>index</em>d parameter of this packet.
	 */
	const param& get_param(unsigned int index) const;

	/** Returns the amount of parameters of this packet
	 */
	unsigned int get_param_count() const;

	/** Returns the raw packet string to send it over the net. This function
	 * is used by net6::connection, but you will most certainly not need it.
	 */
	std::string get_raw_string() const;

	/** Creates a packet out of a raw packet string got from antoher host.
	 * This function is used by net6::connection, but you will most
	 * certainly not need it.
	 */
	void set_raw_string(const std::string& raw_string);

protected:
	static std::string escape(const std::string& string);
	static std::string unescape(const std::string& string);

	void set_raw_param(const std::string& param_string);

	std::string command;
	std::vector<param> params;
	unsigned int prio;
};

}

#endif

