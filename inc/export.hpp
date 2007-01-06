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

#ifndef _NET6_EXPORT_HPP_
#define _NET6_EXPORT_HPP_

#ifndef NET6_EXPORT
# ifdef WIN32
#  ifdef COMPILING_NET6
#   ifdef BUILDING_DLL
#    define NET6_EXPORT __declspec(dllexport)
#   else // BUILDING_DLL
#    define NET6_EXPORT // !BUILDING_DLL
#   endif
#  else // COMPILING_NET6
#   ifndef USE_NET6_STATIC
#    define NET6_EXPORT __declspec(dllimport)
#   else // USE_NET6_STATIC
#    define NET6_EXPORT
#   endif // !USE_NET6_STATIC
#  endif // !COMPILING_NET6
# else // WIN32
#  define NET6_EXPORT
# endif // !WIN32 
#endif // NET6_EXPORT

#endif

