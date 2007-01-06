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

#include <cassert>
#include "error.hpp"
#include "connection.hpp"

net6::connection::connection(const address& addr)
 : offset(0), remote_sock(addr), remote_addr(addr.clone() ), part_pack(false)
{
	remote_sock.read_event().connect(
		sigc::mem_fun(*this, &connection::on_sock_event) );
	remote_sock.write_event().connect(
		sigc::mem_fun(*this, &connection::on_sock_event) );
	remote_sock.error_event().connect(
		sigc::mem_fun(*this, &connection::on_sock_event) );
}

net6::connection::connection(const tcp_client_socket& sock, const address& addr)
 : offset(0), remote_sock(sock), remote_addr(addr.clone() ), part_pack(false)
{
	remote_sock.read_event().connect(
		sigc::mem_fun(*this, &connection::on_sock_event) );
	remote_sock.write_event().connect(
		sigc::mem_fun(*this, &connection::on_sock_event) );
	remote_sock.error_event().connect(
		sigc::mem_fun(*this, &connection::on_sock_event) );
}

net6::connection::~connection()
{
	delete remote_addr;
}

const net6::address& net6::connection::get_remote_address() const
{
	return *remote_addr;
}

const net6::tcp_client_socket& net6::connection::get_socket() const
{
	return remote_sock;
}

void net6::connection::send(const packet& pack)
{
	std::list<packet>::iterator iter = packet_queue.begin();

	// Do not push the packet to the front of the queue if the first
	// packet has been sent partially already.
	if(part_pack) ++ iter;
	
	// Insert into list
	for(; iter != packet_queue.end(); ++ iter)
	{
		// Is the new priority higher than the one of this packet?
		// Insert before in order to be sent before this one.
		if(pack.get_priority() > iter->get_priority() )
		{
			packet_queue.insert(iter, pack);
			return;
		}
	}

	// Not inserted already? Push back.
	packet_queue.push_back(pack);
}

unsigned int net6::connection::send_queue_size() const
{
	return static_cast<unsigned int>(packet_queue.size() );
}

net6::connection::signal_recv_type net6::connection::recv_event() const
{
	return signal_recv;
}

net6::connection::signal_send_type net6::connection::send_event() const
{
	return signal_send;
}

net6::connection::signal_close_type net6::connection::close_event() const
{
	return signal_close;
}

void net6::connection::on_sock_event(socket& sock, socket::condition io) try
{
	tcp_client_socket& tcp_sock = static_cast<tcp_client_socket&>(sock);
	if(io & socket::INCOMING)
	{
		// Get up to 1024 bytes
		char buffer[1024 + 1];
		socket::size_type bytes = tcp_sock.recv(buffer, 1024);
		if(bytes <= 0)
		{
			signal_close.emit();
		}
		else
		{
			buffer[bytes] = '\0';
			std::string::size_type pos = recv_data.length();
			recv_data += buffer;

			// First store the packet strings in a seperate list to
			// allow signal handlers to delete the connection object
			std::list<std::string> packet_list;

			// Packets are seperated by new lines
			while( (pos = recv_data.find('\n', pos)) !=
			       std::string::npos)
			{
				std::string packet_string;
				packet_string = recv_data.substr(0, pos + 1);
				recv_data.erase(0, pos + 1);
				pos = 0;
				packet_list.push_back(packet_string);
			}

			// Build the packets now: We do not depend on recv_data
			// anymore which may be deleted if a signal handler
			// deletes the connection object (e.g. as a result of a
			// login_failed packet or something).
			std::list<std::string>::iterator iter;
			for(iter = packet_list.begin();
			    iter != packet_list.end();
			    ++ iter)
			{
				packet pack;
				pack.set_raw_string(*iter);
				signal_recv.emit(pack);
			}
		}
	}

	if(io & socket::OUTGOING)
	{
		assert(packet_queue.begin() != packet_queue.end() );

		std::string string = packet_queue.begin()->get_raw_string();
		const char* data = string.c_str() + offset;

		part_pack = true;
		socket::size_type bytes;
		bytes = tcp_sock.send(data, string.length() - offset);
		if(bytes <= 0)
		{
			signal_close.emit();
		}
		else
		{
			offset += bytes;

			// Wrote whole packet?
			if(offset == string.length() )
			{
				packet send_pack = *packet_queue.begin();
				packet_queue.erase(packet_queue.begin() );
				signal_send.emit(send_pack);

				offset = 0;
				part_pack = false;
			}
		}
	}

	if(io & socket::IOERROR)
	{
		signal_close.emit();
	}
}
catch(net6::error& e)
{
	if(e.get_code() == error::CONNECTION_RESET ||
	   e.get_code() == error::BROKEN_PIPE)
		signal_close.emit();
	else
		throw e;
}
