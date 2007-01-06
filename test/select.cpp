
#include <iostream>

#include <net6/main.hpp>
#include <net6/address.hpp>
#include <net6/socket.hpp>
#include <net6/select.hpp>

const int port = 1349;
bool quit = false;
net6::selector selector;
net6::socket* gsock;

int client_main(int argc, char* argv[]);

void on_server_io(net6::io_condition io)
{
	net6::tcp_client_socket& tcp_sock =
		static_cast<net6::tcp_client_socket&>(*gsock);

	if(io & net6::IO_INCOMING)
	{
		char buffer[1024 + 1];
		unsigned int bytes = tcp_sock.recv(buffer, 1024);

		if(bytes == 0)
		{
			std::cout << "Connection lost" << std::endl;
			quit = true;
		}
		else
		{
			buffer[bytes] ='\0';
			std::cout << "Got " << buffer << std::endl;
		}
	}
	if(io & net6::IO_OUTGOING)
	{
		tcp_sock.send("Hallo, Welt!", 12);
		selector.set(
			tcp_sock,
			selector.get(tcp_sock) & ~net6::IO_OUTGOING
		);
	}
	if(io & net6::IO_ERROR)
	{
		std::cout << "Another error occured" << std::endl;
		quit = true;
	}
}

void on_client_io(net6::io_condition io)
{
	net6::tcp_client_socket& tcp_sock =
		static_cast<net6::tcp_client_socket&>(*gsock);

	if(io & net6::IO_INCOMING)
	{
		char buffer[1024 + 1];
		int bytes = tcp_sock.recv(buffer, 1024);

		if(bytes == 0)
		{
			std::cout << "Connection lost" << std::endl;
			quit = true;
		}
		else
		{
			buffer[bytes] ='\0';
			std::cout << "Got " << buffer << std::endl;
		}
	}
	if(io & net6::IO_OUTGOING)
	{
		tcp_sock.send("Foobar!", 7);
		selector.set(
			tcp_sock,
			selector.get(tcp_sock) & ~net6::IO_OUTGOING
		);
	}
	if(io & net6::IO_ERROR)
	{
		std::cout << "Another error occured" << std::endl;
		quit = true;
	}
}


int main(int argc, char* argv[]) try
{
	net6::main kit;

	if(argc > 1)
		return client_main(argc, argv);

	net6::ipv4_address serv_addr(port);
	net6::tcp_server_socket sock(serv_addr);

	net6::ipv4_address client_addr;
	std::auto_ptr<net6::tcp_client_socket> client =
		sock.accept(client_addr);

	gsock = client.get();

	std::cout << "Connection from " << client_addr.get_name() << std::endl;

	client->io_event().connect(sigc::ptr_fun(&on_server_io) );

	selector.set(
		*client,
		net6::IO_INCOMING | net6::IO_OUTGOING | net6::IO_ERROR
	);

	while(!quit)
		selector.select();

	return 0;
}
catch(std::exception& e)
{
	std::cerr << e.what() << std::endl;
}

int client_main(int argc, char* argv[])
{
	net6::ipv4_address client_addr =
		net6::ipv4_address::create_from_hostname("localhost", port);

	net6::tcp_client_socket client(client_addr);
	gsock = &client;

	client.io_event().connect(sigc::ptr_fun(&on_client_io) );

	selector.set(
		client,
		net6::IO_INCOMING | net6::IO_OUTGOING | net6::IO_ERROR
	);

	while(!quit)
		selector.select();
	
	return 0;
}
