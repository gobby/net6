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

#include <map>
#include <string>
#include <vector>
#include <sstream>
#include <typeinfo>
#include <stdexcept>
#include <sigc++/slot.h>

namespace net6
{

/** Basic packet parameter type. This type can hold a parameter of any type.
 * Normally, you do not have to create those objects yourself, net6 manages
 * them.
 */
class basic_parameter {
public:
	/** Exception class that is thrown by basic_parameter::as if the
	 * requested type is not the stored one.
	 */
	class bad_type : public std::runtime_error {
	public:
		bad_type()
		 : std::runtime_error("Bad type") { }
	};

	/** Exception class that is thrown by packet::get_param if the
	 * requested parameter index is out of range.
	 */
	class bad_count : public std::runtime_error {
	public:
		bad_count()
		 : std::runtime_error("Bad count") { }
	};

	/** Exception class that may be thrown by the callback of the
	 * packet type map if the parameter has a bad format.
	 */
	class bad_format : public std::runtime_error {
	public:
		bad_format(const std::string& reason)
		 : std::runtime_error(reason) { }
	};

	/** Exception that might be thrown by a packet reception handler if
	 * the packet has invalid parameters.
	 */
	class bad_value : public std::runtime_error {
	public:
		bad_value(const std::string& error_message)
		 : std::runtime_error(error_message) { }
	};

protected:
	/** Base class for net6::basic_parameter::data.
	 */
	class basic_data {
	public:
		virtual ~basic_data() { }

		/** Returns the type of the value stored in the data object.
		 */
		virtual const std::type_info& type() const = 0;

		/** Creates a copy of the data object.
		 */
		virtual basic_data* clone() const = 0;
	};

	/** Data class for holding an object of any type.
	 */
	template<typename data_type>
	class data : public basic_data {
	public:
		data(const data_type& value)
		 : m_value(value) { }

		/** Returns the type of the value stored in the data object.
		 */
		virtual const std::type_info& type() const {
			return typeid(data_type);
		}

		/** Creates a copy of the data object.
		 */
		virtual basic_data* clone() const {
			return new data<data_type>(m_value);
		}

		/** Returns the value of this object.
		 */
		const data_type& value() const {
			return m_value;
		}
	protected:
		data_type m_value;
	};
public:
	/** Type to identify types on the network layer.
	 */
	typedef char identification_type;

	/** Creates a new basic_parameter with the given type id and the
	 * given value.
	 */
	template<typename data_type>
	basic_parameter(identification_type type_id, const data_type& value)
	 : m_data(new data<data_type>(value) ), m_type_id(type_id) { }

	/** Creates a copy of this basic parameter.
	 */
	virtual basic_parameter* clone() const = 0;

	virtual ~basic_parameter() {
		delete m_data;
	}

	/** Returns the value of the parameter which must have the given
	 * type. Otherwise, bad_type is thrown.
	 */
	template<typename data_type>
	const data_type& as() const {
		if(m_data->type() != typeid(data_type) )
			throw bad_type();
		return static_cast<data<data_type>*>(m_data)->value();
	}

	/** Returns the ID for the type of this parameter.
	 */
	char get_type_id() const {
		return m_type_id;
	}

	/** Returns a string representation of the object.
	 */
	virtual std::string to_string() const = 0;

protected:
	char m_type_id;
	basic_data* m_data;
};

/** Packet parameter. To create your own parameter types, you have to create
 * a specialised version of this class with your type and overwrite
 * basic_parameter::clone and basic_parameter::to_string. The type_id that
 * is given to the base class (basic_parameter) must not be in use by another
 * type. net6 occupies 'b', 'i', 's' and 'f'.
 *
 * Finally call packet::register_type
 * with this ID and a function that converts the string representation of the
 * type back to a parameter object. This function may throw
 * basic_parameter::bad_format if the string has a bad format that does not
 * correspond to the desired type. Look at inc/packet.hpp for examples.
 */
template<typename data_type>
class parameter : public basic_parameter {
public:
	parameter(char type_id, const data_type& type)
	 : basic_parameter(type_id, type) { }

	virtual basic_parameter* clone() const {
		return new parameter<data_type>(as<data_type>() );
	}
};

/** Boolean parameter.
 */
template<>
class parameter<bool> : public basic_parameter {
public:
	parameter(bool value)
	 : basic_parameter(TYPE_ID, value) { }

	virtual basic_parameter* clone() const {
		return new parameter<bool>(as<bool>() );
	}

	virtual std::string to_string() const {
		return as<bool>() ? "1" : "0";
	}

	static basic_parameter* from_string(const std::string& str) {
		if(str == "1") return new parameter<bool>(true);
		if(str == "0") return new parameter<bool>(false);

		throw basic_parameter::bad_format(
			"Boolean value is neither 0 nor 1"
		);
	}

	static const identification_type TYPE_ID = 'b';
};

/** Integer parameter.
 */
template<>
class parameter<int> : public basic_parameter {
public:
	parameter(int value)
	 : basic_parameter(TYPE_ID, value) { }

	virtual basic_parameter* clone() const {
		return new parameter<int>(as<int>() );
	}

	virtual std::string to_string() const {
		std::stringstream stream;
		stream << std::hex << as<int>();
		return stream.str();
	}

	static basic_parameter* from_string(const std::string& str) {
		std::stringstream stream(str);
		int result;
		stream >> std::hex >> result;

		if(stream.bad() )
			throw basic_parameter::bad_format(
				"Not a hexadecimal integer"
			);

		return new parameter<int>(result);
	}

	static const identification_type TYPE_ID = 'i';
};

/** Unsigned integer parameter. This is just a wrapper for the int parameter to
 * allow users to call packet::operator<< with unsigned integers. Be sure to
 * use as<int> to retrieve the correct parameter type even if storing
 * unsigned integers in a packet.
 */
template<>
class parameter<unsigned int> : public parameter<int> {
public:
	parameter(unsigned int value)
	 : parameter<int>(static_cast<int>(value) ) { }
};

/** Same for long and unsigned long.
 * TODO: Make long the default type because a long value may be truncated if
 * it exceeds the maximum size of an int.
 */
template<>
class parameter<long> : public parameter<int> {
public:
	parameter(long value)
	 : parameter<int>(static_cast<int>(value) ) { }
};

template<>
class parameter<unsigned long> : public parameter<unsigned int> {
public:
	parameter(unsigned long value)
	 : parameter<unsigned int>(static_cast<unsigned int>(value) ) { }
};

/** String parameter.
 */
template<>
class parameter<std::string> : public basic_parameter {
public:
	parameter(const std::string& value)
	 : basic_parameter(TYPE_ID, value) { }

	virtual basic_parameter* clone() const {
		return new parameter<std::string>(as<std::string>() );
	}

	virtual std::string to_string() const {
		return as<std::string>();
	}

	static basic_parameter* from_string(const std::string& str) {
		return new parameter<std::string>(str);
	}

	static const identification_type TYPE_ID = 's';
};

/** Wrapper for the string parameter to allow users to call packet::operator<<
 * with const char*.
 */
template<>
class parameter<const char*> : public parameter<std::string> {
public:
	parameter(const char* value)
	 : parameter<std::string>(value) { }
};

/** Wrapper for the string parameter to allow usres to call packet::operator<<
 * with an array of chars.
 */
template<int N>
class parameter<char[N]> : public parameter<std::string> {
public:
	parameter(const char* value)
	 : parameter<std::string>(value) { }
};

/** Double packet parameter.
 */
template<>
class parameter<double> : public basic_parameter {
public:
	parameter(double value)
	 : basic_parameter(TYPE_ID, value) { }

	virtual basic_parameter* clone() const {
		return new parameter<double>(as<double>() );
	}

	virtual std::string to_string() const {
		std::stringstream stream;
		stream << as<double>();
		return stream.str();
	}

	static basic_parameter* from_string(const std::string& str) {
		std::stringstream stream(str);
		double val;
		stream >> val;

		if(stream.bad() )
			throw basic_parameter::bad_format(
				"Not a floating point value"
			);

		return new parameter<double>(val);
	}

	static const identification_type TYPE_ID = 'f';
};

/** Wrapper for float data types.
 */
template<>
class parameter<float> : public parameter<double> {
public:
	parameter(float value)
	 : parameter<double>(value) { }
};

/** High-level object that represents a packet that may be sent over the
 * network. A packet exists of a command and a variable amount of parameters
 * with variable type.
 */

class packet
{
public:
	typedef basic_parameter::identification_type identification_type;
	typedef sigc::slot<basic_parameter*, std::string> type_lookup_slot;
	typedef std::map<identification_type, type_lookup_slot> type_lookup_map;

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
	template<typename T> packet& operator<<(const T& val) {
		params.push_back(new parameter<T>(val) );
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
	const basic_parameter& get_param(unsigned int index) const;

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

	/** Registers a new type. If you decide to write a parameter for a type
	 * that is not already supported by net6 (which, in fact, is bool, int,
	 * float and std::string) you have to call this function with the
	 * type ID you chose (see net6::parameter) and a function which
	 * translates a string back to an object of the given type.
	 */
	static void register_type(identification_type type,
	                          type_lookup_slot slot);

protected:
	static std::string escape(const std::string& string);
	static std::string unescape(const std::string& string);

	void set_raw_param(const std::string& param_string);

	std::string command;
	std::vector<basic_parameter*> params;
	unsigned int prio;

	static type_lookup_map type_map;
};

}

#endif

