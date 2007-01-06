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

#ifndef _NET6_DEFAULT_ACCUMULATOR_HPP_
#define _NET6_DEFAULT_ACCUMULATOR_HPP_

namespace net6
{

/** Accumulator for signals with return type that defaults to a value if no
 * signal handler is connected.
 */

template<typename return_type, return_type default_return>
class default_accumulator {
public:
	typedef return_type result_type;

	template<typename iterator>
	result_type operator()(iterator begin, iterator end) const {
		return_type result = default_return;
		for(; begin != end; ++ begin)
			result = *begin;
		return result;
	}
};

}

#endif // _NET6_DEFAULT_ACCUMULATOR_HPP_
