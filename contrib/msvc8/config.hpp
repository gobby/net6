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

#ifndef _NET6_CONTRIB_MSVC8_CONFIG_HPP_
#define _NET6_CONTRIB_MSVC8_CONFIG_HPP_

#ifndef _MSC_VER
#error this <inttypes.h> replacement solely targets MSVC8
#endif

#include <winsock2.h>
#include <ws2tcpip.h>

/** this win32/msvc8-specific implementation of inet_ntop supports only
 * the AF_INET address type.
 */
inline const char *inet_ntop(int af, const void *__restrict src, char *__restrict dest, socklen_t size)
{
  // ipv6 not supported
  if(AF_INET!=af)
  {
    printf(__FUNCTION__ " is only implemented for AF_INET address family on win32/msvc8");
    abort();
  }

  // format address
  char *s=inet_ntoa(*reinterpret_cast<const in_addr*>(src));
  if(!s)
    return 0;

  // copy to given buffer
  socklen_t len=(socklen_t)strlen(s);
  if(len>=size)
    return 0;
  return strncpy(dest, s, len);
}

#endif