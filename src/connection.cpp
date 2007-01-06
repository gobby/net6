/* net6 - library providing ipv4/ipv6 network access
 * Copyright (C) 2005 Armin Burgmeier / 0x539 dev group
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "connection.hpp"

net6::connection::connection(const address& addr)
 : offset(0), remote_sock(addr), remote_addr(addr.clone() )
{
	remote_sock.read_event().connect(
		sigc::mem_fun(*this, &connection::on_sock_event) );
	remote_sock.write_event().connect(
		sigc::mem_fun(*this, &connection::on_sock_event) );
	remote_sock.error_event().connect(
		sigc::mem_fun(*this, &connection::on_sock_event) );
}

net6::connection::connection(const tcp_client_socket& sock, const address& addr)
 : offset(0), remote_sock(sock), remote_addr(addr.clone() )
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
	packet_queue.push(pack);
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

void net6::connection::on_sock_event(socket& sock, socket::condition io)
{
	tcp_client_socket& tcp_sock = static_cast<tcp_client_socket&>(sock);
	if(io & socket::IN)
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

			// Packets are seperated by new lines
			while( (pos = recv_data.find('\n', pos)) !=
			       std::string::npos)
			{
				std::string packet_string;
				packet_string = recv_data.substr(0, pos + 1);
				recv_data.erase(0, pos + 1);
				pos = 0;

				packet pack;
				pack.set_raw_string(packet_string);
				signal_recv.emit(pack);
			}
		}
	}

	if(io & socket::OUT)
	{
		std::string string = packet_queue.front().get_raw_string();
		const char* data = string.c_str() + offset;

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
				packet send_pack = packet_queue.front();
				packet_queue.pop();
				signal_send.emit(send_pack);

				offset = 0;
			}
		}
	}

	if(io & socket::ERR)
	{
		signal_close.emit();
	}
}
