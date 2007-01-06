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

#ifndef _NET6_PACKET_HPP_
#define _NET6_PACKET_HPP_

#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>
#include "serialise.hpp"
#include "connection.hpp"

namespace net6
{

/** Exception class that is thrown by packet::get_param if the
 * requested parameter index is out of range.
 */
class bad_count: public std::runtime_error {
public:
	bad_count():
		std::runtime_error("Bad count") { }
};

/** Exception class that is thrown when a parameter has an illegal format for
 * the requested type.
 */
class bad_format: public std::runtime_error {
public:
	bad_format(const std::string& reason):
		std::runtime_error(reason) { }
};

/** Exception that might be thrown by a packet reception handler if
 * the packet has invalid parameters.
 */
class bad_value: public std::runtime_error {
public:
	bad_value(const std::string& error_message):
		std::runtime_error(error_message) { }
};

/** Packet parameter.
 */
class parameter
{
public:
	/** Uses the given string as serialised parameter data.
	 */
	parameter(const std::string& value);

	/** Serialises the object of <em>data_type</em> and serialises it
	 * through <em>ctx</em> as parameter value.
	 */
	template<typename data_type>
	parameter(const data_type& type,
	          const serialise::context<data_type>& ctx =
	          serialise::hex_context<data_type>());

	/** Returns the serialised data.
	 */
	const std::string& serialised() const;

	/** Deserialises the parameter value with the given context.
	 */
	template<typename data_type>
	data_type as(const serialise::context<data_type>& ctx =
	             serialise::hex_context<data_type>()) const;

protected:
	serialise::data m_value;
};

template<typename data_type>
parameter::parameter(const data_type& type,
                     const serialise::context<data_type>& ctx):
	m_value(type, ctx)
{
}

template<typename data_type>
data_type parameter::as(const serialise::context<data_type>& ctx) const
{
	try
	{
		// Deserialise value
		return m_value.as<data_type>(ctx);
	}
	catch(serialise::conversion_error& e)
	{
		// Convert error to bad_format in net6 network layer
		throw bad_format(e.what() );
	}
}

/** High-level object that represents a packet that may be sent over the
 * network. A packet exists of a command and a variable amount of parameters
 * with variable type.
 */
class packet
{
public:
	/** Thrown by packet::packet(connection::queue&) if there is no
	 * complete packet in the queue.
	 */
	class end_of_queue: public std::runtime_error
	{
	public:
		end_of_queue():
			std::runtime_error("No complete packet in queue") { }
	};

	/** Creates a new packet.
	 * @param command Command for the packet.
	 * @param size Number of parameters the packet will preallocate memory
	 * for.
	 */
	packet(const std::string& command,
	       unsigned int size = 0);

	/** Reads a packet from a connection queue. end_of_queue is thrown if
	 * there is only a partial packet on the queue. This constructor is used
	 * by net6::connection to split incoming data into separate packets.
	 * You will most certainly not need it.
	 */
	packet(connection::queue& queue);

	/** Adds a new parameter to the packet.
	 */
	template<typename data_type>
	void add_param(const data_type& value,
	               const serialise::context<data_type>& ctx =
	               serialise::hex_context<data_type>() );

	/** Shortcut for add_param(T)
	 */
	template<typename data_type>
	packet& operator<<(const data_type& value);

	/** Returns the command of this packet.
	 */
	const std::string& get_command() const;

	/** Returns the <em>index</em>d parameter of this packet.
	 */
	const parameter& get_param(unsigned int index) const;

	/** Returns the amount of parameters of this packet
	 */
	unsigned int get_param_count() const;

	/** Pushes this packet onto the given connection queue. This function
	 * is used by net6::connection to enqueue a packet for sending it to
	 * a remote host. You will most certainly not need it.
	 */
	void enqueue(connection::queue& queue) const;
protected:
	static std::string escape(const std::string& string);
	static std::string unescape(const std::string& string);

	std::string command;
	std::vector<parameter> params;
};

template<typename data_type>
void packet::add_param(const data_type& value,
                       const serialise::context<data_type>& ctx)
{
	params.push_back(parameter(value, ctx) );
}

template<typename data_type>
packet& packet::operator<<(const data_type& value)
{
	// Add parameter with default context
	add_param(value);
}

}

#endif

