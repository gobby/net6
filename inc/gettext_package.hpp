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

#ifndef _NET6_GETTEXT_PACKAGE_HPP_
#define _NET6_GETTEXT_PACKAGE_HPP_

#include <string>
#include "non_copyable.hpp"

namespace net6
{

class gettext_package: private non_copyable
{
public:
	/** Initialises GNU gettext to load translations for the package
	 * <em>package</em> from <em>locale_dir</em>. On Microsoft Windows, the
	 * function tries to load it from a locale/ subfolder of the directory
	 * where the executable is located. There is no common shared folder
	 * like /usr/share/locale on UNIX-like systems.
	 */
	gettext_package(const std::string& package,
	                const std::string& locale_dir);

	/** Returns a translated string for the given text.
	 */
	const char* gettext(const char* text) const;

	/** Returns a translated string depending, either a singular or a
	 * plural form, depending on the number <em>n</em>.
	 */
	const char* ngettext(const char* msgid,
	                     const char* msgid_plural,
	                     unsigned long int n) const;
protected:
	std::string m_package;
};

}

#endif // _NET6_GETTEXT_PACKAGE_HPP_
