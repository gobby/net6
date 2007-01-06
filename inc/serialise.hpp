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

#ifndef _NET6_SERIALISE_HPP_
#define _NET6_SERIALISE_HPP_

#include <string>
#include <sstream>
#include <stdexcept>

/** Generic stuff to de/serialise data types to/from strings.
 */
namespace serialise
{

/** Error that is thrown if conversion from a string fails. For example, if
 * "t3" should be converted to int.
 */
class conversion_error: public std::runtime_error
{
public:
	conversion_error(const std::string& message);
};

/** Context to convert a specified type to or from a string.
 */
template<typename data_type>
class context
{
public:
	/** Converts something to a string.
	 */
	virtual std::string to_string(const data_type& from) const;

	/** Converts a string back to the data type of this context. May
	 * throw a conversion error if the string is illegally formatted for
	 * the result type.
	 */
	virtual data_type from_string(const std::string& string) const;

protected:
	/** Internal function that is called before data is converted. Derived
	 * classes may use this to modify stream stuff to format output.
	 */
	virtual void on_stream_setup(std::stringstream& stream) const;
};

/** Special context that uses a hexadecimal representation for numerical types.
 */
template<typename data_type>
class hex_context: public context<data_type>
{
protected:
	/** Internal function that is called before data is converted. Derived
	 * classes may use this to modify stream stuff to format output.
	 */
	virtual void on_stream_setup(std::stringstream& stream) const;
};

/** String serialisation does not need any conversions.
 */
template<>
class context<std::string>
{
public:
	typedef std::string data_type;

	// TODO: Return const string ref here?
	virtual std::string to_string(const data_type& from) const;
	virtual data_type from_string(const std::string& string) const;
};

/** const char* serialisation is only supported in one direction
 * (const char* -> string). If you want to get certain data as const char*,
 * get it as std::string and call c_str() on it.
 */
template<>
class context<const char*>
{
public:
	typedef const char* data_type;

	virtual std::string to_string(const data_type& from) const;
};

/** char array serialisation is only supported in one direction
 * (char[N] -> string). If you want to get certain data as a char array, get
 * it as std::string and call c_str() on it.
 */
template<size_t N>
class context<char[N]>: public context<std::string>
{
public:
	typedef const char data_type[N];

	virtual std::string to_string(const data_type& from) const;
};

/** A serialised object.
 */
class data
{
public:
	/** Uses the given string as serialised data without converting it.
	 */
	data(const std::string& serialised);

	/** Serialises the given object with the given context. A default
	 * context is used if no one is given.
	 */
	template<typename type>
	data(const type& data, const context<type>& ctx = context<type>());

	/** Returns the serialised data.
	 */
	const std::string& serialised() const;

	/** Deserialises the object with the given context. A default context
	 * is used of no one is given.
	 */
	template<typename type>
	type as(const context<type>& ctx = context<type>()) const;

protected:
	std::string m_serialised;
};

template<typename data_type>
std::string context<data_type>::to_string(const data_type& data) const
{
	std::stringstream stream;
	on_stream_setup(stream);
	stream << data;
	return stream.str();
}

template<typename data_type>
data_type context<data_type>::from_string(const std::string& string) const
{
	std::stringstream stream(string);
	on_stream_setup(stream);
	data_type data;
	stream >> data;

	if(stream.bad() )
		throw conversion_error("Type conversion failed");

	return data;
}

template<typename data_type>
void context<data_type>::on_stream_setup(std::stringstream& stream) const
{
}

template<typename data_type>
void hex_context<data_type>::on_stream_setup(std::stringstream& stream) const
{
	stream >> std::hex;
}

template<typename data_type>
data::data(const data_type& data, const context<data_type>& ctx):
	m_serialised(ctx.to_string(data) )
{
}

template<typename data_type>
data_type data::as(const context<data_type>& ctx) const
{
	return ctx.from_string(m_serialised);
}

template<size_t N>
std::string context<char[N]>::to_string(const data_type& from) const
{
	return from;
}

} // namespace serialise

#endif // _NET6_SERIALISE_HPP_

