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

/* This file must not be used in other header files. */

#include "config.hpp"

#ifdef ENABLE_NLS
#include <libintl.h>

template<typename char_type>
inline const char_type* _(const char_type* text) {
	return dgettext(PACKAGE, text);
}
#else
/* This functions is a no-op for systems without a useable GNU gettext. */
template<typename char_type>
inline const char_type* _(const char_type* text) {
	return text;
}
#endif

