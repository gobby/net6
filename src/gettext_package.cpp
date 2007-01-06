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

#include "config.hpp"
#include "gettext_package.hpp"

#ifdef WIN32
# include <windows.h>
#endif

#ifdef ENABLE_NLS

#include <libintl.h>

net6::gettext_package::gettext_package(const std::string& package,
                                       const std::string& localedir):
	m_package(package)
{
	const char* locale_dir = localedir.c_str();
#ifdef WIN32
	char buf[256 + 1];
	GetModuleFileNameA(NULL, buf, 256);
	char* last_sep = strrchr(buf, '\\');
	strcpy(last_sep + 1, "locale");
	locale_dir = buf;
#endif

	bindtextdomain(m_package.c_str(), locale_dir);
	bind_textdomain_codeset(m_package.c_str(), "UTF-8");
}

const char* net6::gettext_package::gettext(const char* msgid) const
{
	return ::dgettext(m_package.c_str(), msgid);
}

const char* net6::gettext_package::ngettext(const char* msgid,
                                            const char* msgid_plural,
                                            unsigned long int n) const
{
	return ::dngettext(m_package.c_str(), msgid, msgid_plural, n);
}

#else // ENABLE_NLS

/* Without NLS support we compile stubs into the library, to prevent
 * linker failure because of missing symbols. Packages using
 * this gettext infrastucture will have their NLS support disabled
 * silently.
 */

net6::gettext_package::gettext_package(const std::string& package,
                                       const std::string& localedir)
{
}

const char* net6::gettext_package::gettext(const char* msgid) const
{
	return msgid;
}

const char* net6::gettext_package::ngettext(const char* msgid,
                                            const char* msgid_plural,
                                            unsigned long int n) const
{
	// The incoming message strings are in English, which only
	// uses the singular form for one item.
	if(n != 1)
		return msgid_plural;
	else
		return msgid;
}

#endif // !ENABLE_NLS

