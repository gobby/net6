/* net6 - Library providing IPv4/IPv6 network access
 * Copyright (C) 2005, 2006 Armin Burgmeier / 0x539 dev group
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _NET6_ENUM_OPS_HPP_
#define _NET6_ENUM_OPS_HPP_

/** Defines bitwise operators for an enumerated type to allow to combine them.
 */
#define NET6_DEFINE_ENUM_OPS(type)                                           \
	inline type operator|(type lhs, type rhs)                            \
	{                                                                    \
		return static_cast<type>(                                    \
			static_cast<int>(lhs) | static_cast<int>(rhs)        \
		);                                                           \
	}                                                                    \
	                                                                     \
	inline type operator&(type lhs, type rhs)                            \
	{                                                                    \
		return static_cast<type>(                                    \
			static_cast<int>(lhs) & static_cast<int>(rhs)        \
		);                                                           \
	}                                                                    \
                                                                             \
	inline type operator^(type lhs, type rhs)                            \
	{                                                                    \
		return static_cast<type>(                                    \
			static_cast<int>(lhs) ^ static_cast<int>(rhs)        \
		);                                                           \
	}                                                                    \
                                                                             \
	inline type& operator|=(type& lhs, type rhs)                         \
	{                                                                    \
		return lhs = (lhs | rhs);                                    \
	}                                                                    \
                                                                             \
	inline type& operator&=(type& lhs, type rhs)                         \
	{                                                                    \
		return lhs = (lhs & rhs);                                    \
	}                                                                    \
                                                                             \
	inline type& operator^=(type& lhs, type rhs)                         \
	{                                                                    \
		return lhs = (lhs ^ rhs);                                    \
	}                                                                    \
                                                                             \
	inline type operator~(type rhs)                                      \
	{                                                                    \
		return static_cast<type>(~static_cast<int>(rhs) );           \
	}

#endif // _NET6_ENUM_OPS_HPP_
