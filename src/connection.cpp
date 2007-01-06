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

#include <iostream>
#include "error.hpp"
#include "packet.hpp"
#include "connection.hpp"

namespace
{
	const net6::connection::queue::size_type INVALID_POS =
		static_cast<net6::connection::queue::size_type>(~0);
};

// We use malloc for the dynamic queue data to be available to use realloc
// if the data exceeds the size of the queue.
net6::connection::queue::queue()
 : data(static_cast<char*>(std::malloc(1024)) ), size(0), alloc(1024)
{
}

net6::connection::queue::~queue()
{
	std::free(data);
}

net6::connection::queue::size_type net6::connection::queue::get_size() const
{
	return size;
}

net6::connection::queue::size_type net6::connection::queue::packet_size() const
{
	for(size_type i = 0; i < size; ++ i)
		if(data[i] == '\n')
			return i;
	return size;
}

const char* net6::connection::queue::get_data() const
{
	return data;
}

void net6::connection::queue::append(const char* new_data, size_type len)
{
	// TODO: This function should raise an exception if the queue
	// exceed a defined default size. This would, however, restrict
	// the maximum packet size. The client could be advised to disconnect
	// the offending client.
	if(size + len > alloc)
	{
		alloc = size + len;
		data = static_cast<char*>(std::realloc(data, alloc *= 2) );
	}

	std::memcpy(data + size, new_data, len);
	size += len;
}

void net6::connection::queue::remove(size_type len)
{
	// TODO: Free a part of the allocated memory when only a half is used,
	// or so.
	if(len > size)
		throw std::logic_error("net6::connection::queue::remove");

	std::memmove(data, data + len, size - len);
	size -= len;
}

net6::connection::connection(const address& addr):
	remote_sock(new tcp_client_socket(addr) ),
	encrypted_sock(NULL),
	remote_addr(addr.clone() ),
	block_pos(INVALID_POS)
{
	remote_sock->io_event().connect(
		sigc::mem_fun(*this, &connection::on_sock_event) );
}

net6::connection::connection(std::auto_ptr<tcp_client_socket> sock,
                             const address& addr):
	remote_sock(sock), encrypted_sock(NULL),
	remote_addr(addr.clone() ),
	block_pos(INVALID_POS)
{
	remote_sock->io_event().connect(
		sigc::mem_fun(*this, &connection::on_sock_event) );
}

const net6::address& net6::connection::get_remote_address() const
{
	return *remote_addr;
}

const net6::tcp_client_socket& net6::connection::get_socket() const
{
	return *remote_sock;
}

void net6::connection::send(const packet& pack)
{
	pack.enqueue(sendqueue);
	if(pack.get_command() == "net6_encryption" ||
	   pack.get_command() == "net6_encryption_ok")
	{
		// All unencrypted traffic needs to go to the peer
		// first, then further sending is blocked.
		block_pos = sendqueue.get_size();
		// We need to determine who initiates the TLS handshake.
		master = (pack.get_command() == "net6_encryption_ok");
	}
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

net6::connection::signal_encrypted_type
net6::connection::encrypted_event() const
{
	return signal_encrypted;
}

void net6::connection::on_sock_event(io_condition io)
{
	try
	{
		if(io & IO_INCOMING)
		{
			if(encrypted_sock->is_handshaking() )
			{
				handle_handshake();
				return;
			}

			// Get up to 1024 bytes
			char buffer[1024];
			socket::size_type bytes =
				remote_sock->recv(buffer, 1024);

			if(bytes == 0)
			{
				on_close();
			}
			else
			{
				// Add to queue
				recvqueue.append(buffer, bytes);

				// First store the packets in a separate list
				// to allow signal handlers to delete the
				// connection object
				std::list<packet> packet_list;

				// Add all packets to the list until
				// end of queue is found.
				try
				{
					while(true)
					{
						packet_list.push_back(
							packet(recvqueue)
						);
					}
				}
				catch(packet::end_of_queue&) {}

				// Emit the signal_recv now. Because we do not
				// depend on recvqueue for reading further
				// data, the signal handler may destroy the
				// connection object.
				std::list<packet>::iterator iter;
				for(iter = packet_list.begin();
				    iter != packet_list.end();
				    ++ iter)
				{
					on_recv(*iter);
				}
			}
		}

		if(io & IO_OUTGOING)
		{
			if(encrypted_sock->is_handshaking() )
			{
				handle_handshake();
				return;
			}

			// Send at most block_pos bytes of data. This
			// ensures that bytes after black_pos are not
			// sent

			// Is there something to send?
			if(sendqueue.get_size() == 0 || block_pos == 0)
			{
				throw std::logic_error(
					"net6::connection::on_sock_event"
				);
			}

			// Send data from queue
			queue::size_type len = sendqueue.get_size();
			if(block_pos != INVALID_POS) len = block_pos;

			socket::size_type bytes = remote_sock->send(
				sendqueue.get_data(),
				len
			);

			if(bytes <= 0)
			{
				on_close();
			}
			else
			{
				// Remove the data we successfully sent from
				// the queue
				sendqueue.remove(bytes);

				if(block_pos != INVALID_POS)
					block_pos -= bytes;

				// Emit on_send signal if all available data
				// has been sent
				if(sendqueue.get_size() == 0 || block_pos == 0)
					on_send();
			}
		}

		if(io & IO_ERROR)
		{
			on_close();
		}
	}
	catch(net6::error& e)
	{
		if(e.get_code() == error::CONNECTION_RESET ||
		   e.get_code() == error::BROKEN_PIPE)
		{
			on_close();
		}
		else
		{
			throw e;
		}
	}
}

void net6::connection::on_send()
{
	signal_send.emit();
	if(master && block_pos == 0)
	{
		encrypted_sock = new tcp_encrypted_socket_server(*remote_sock);
		remote_sock.reset(encrypted_sock);
		handle_handshake();
	}
}

void net6::connection::on_recv(const net6::packet& pack)
{
	// This is net6's use of TLS:
	//  > net6_encryption
	//  < net6_encryption_ok
	//  < handshake
	//  > handshake
	if(pack.get_command() == "net6_encryption")
	{
		packet reply("net6_encryption_ok");
		send(reply);
	}
	else if(pack.get_command() == "net6_encryption_ok")
	{
		encrypted_sock =
			new tcp_encrypted_socket_client(*remote_sock);
		remote_sock.reset(encrypted_sock);
		handle_handshake();
	}

	try
	{
		signal_recv.emit(pack);
	}
	catch(net6::bad_count& e)
	{
		std::cerr << "net6 warning: Protocol mismatch! Received bad "
		          << "parameter count from " << remote_addr->get_name()
		          << " in packet " << pack.get_command() << std::endl;
	}
	catch(net6::bad_format& e)
	{
		std::cerr << "net6 warning: Protocol mismatch! Received bad "
		          << "parameter format from " << remote_addr->get_name()
		          << " in packet " << pack.get_command() << ": "
		          << e.what() << std::endl;
	}
	catch(net6::bad_value& e)
	{
		std::cerr << "net6 warning: Protocol mismatch! Received bad "
		          << "parameter value from " << remote_addr->get_name()
		          << " in packet " << pack.get_command() << ": "
		          << e.what() << std::endl;
	}
}

void net6::connection::on_close()
{
	signal_close.emit();
}

void net6::connection::handle_handshake()
{
	if(!encrypted_sock)
		throw std::logic_error(
			"handle_handshake called and no encrypted "
			"socket present"
		);

	if(encrypted_sock->handshake() )
	{
		block_pos = INVALID_POS;
		signal_encrypted.emit(
			(sendqueue.get_size() > 0) ? ENCRYPTED : ENCRYPTED_PENDING
			);
	}
	else
	{
		signal_encrypted.emit(
			encrypted_sock->get_dir() ?
			HANDSHAKING_SEND : HANDSHAKING_RECV
			);
	}
}

