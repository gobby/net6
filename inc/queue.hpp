/* net6 - Library providing IPv4/IPv6 network access
 * Copyright (C) 2005, 2006 Armin Burgmeier / 0x539 dev group
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

#ifndef _NET6_QUEUE_HPP_
#define _NET6_QUEUE_HPP_

#include <cstdlib>
#include "non_copyable.hpp"

namespace net6
{

/** Internal buffer for incoming or outgoing data.
 */
class queue: private non_copyable
{
public:
	typedef std::size_t size_type;
	static const size_type INVALID_POS = ~static_cast<size_type>(0);

	queue();
	~queue();

	/** @brief Unblocks and clears the whole queue.
	 */
	void clear();

	/** Returns the size of the queue.
	 */
	size_type get_size() const;

	/** Returns the size of the next packet in the queue.
	 */
	size_type packet_size() const;

	/** Returns a pointer to the data that is currently enqueued.
	 */
	const char* get_data() const;

	/** Appends new data to the queue.
	 */
	void append(const char* new_data, size_type len);

	/** Removes data from the queue.
	 */
	void remove(size_type len);

	void block();
	void unblock();
private:
	char* data;
	size_type size;
	size_type alloc;
	size_type block_p;
};

} // namespace net6

#endif // _NET6_QUEUE_HPP_
