
#include <iostream>

#include <net6/main.hpp>
#include <net6/address.hpp>
#include <net6/socket.hpp>
#include <net6/select.hpp>

const int port = 1349;
bool quit = false;
net6::selector selector;

int client_main(int argc, char* argv[]);

void on_server_io(net6::socket& sock, net6::socket::condition io)
{
	net6::tcp_client_socket& tcp_sock = static_cast<net6::tcp_client_socket&>(sock);
	if(io & net6::socket::INCOMING)
	{
		char buffer[1024 + 1];
		int bytes = tcp_sock.recv(buffer, 1024);

		if(bytes < 0)
		{
			std::cout << "Error occured" << std::endl;
			quit = true;
		}
		else if(bytes == 0)
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
	if(io & net6::socket::OUTGOING)
	{
		tcp_sock.send("Hallo, Welt!", 12);
		selector.remove(sock, net6::socket::OUTGOING);
	}
	if(io & net6::socket::ERROR)
	{
		std::cout << "Another error occured" << std::endl;
		quit = true;
	}
}

void on_client_io(net6::socket& sock, net6::socket::condition io)
{
	net6::tcp_client_socket& tcp_sock = static_cast<net6::tcp_client_socket&>(sock);
	if(io & net6::socket::INCOMING)
	{
		char buffer[1024 + 1];
		int bytes = tcp_sock.recv(buffer, 1024);

		if(bytes < 0)
		{
			std::cout << "Error occured" << std::endl;
			quit = true;
		}
		else if(bytes == 0)
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
	if(io & net6::socket::OUTGOING)
	{
		tcp_sock.send("Foobar!", 7);
		selector.remove(sock, net6::socket::OUTGOING);
	}
	if(io & net6::socket::ERROR)
	{
		std::cout << "Another error occured" << std::endl;
		quit = true;
	}
}


int main(int argc, char* argv[]) try
{
	net6::ipv4_address test = net6::ipv4_address::create_from_hostname("localhost", 34325);
	net6::address* other = test.clone();
	std::cout << other->get_name() << std::endl;
	delete other;

	net6::main kit;

	if(argc > 1)
		return client_main(argc, argv);

	net6::ipv4_address serv_addr(port);
	net6::tcp_server_socket sock(serv_addr);

	net6::ipv4_address client_addr;
	net6::tcp_client_socket client = sock.accept(client_addr);

	std::cout << "Connection from " << client_addr.get_name() << std::endl;

	client.read_event().connect(sigc::ptr_fun(&on_server_io) );
	client.write_event().connect(sigc::ptr_fun(&on_server_io) );
	client.error_event().connect(sigc::ptr_fun(&on_server_io) );

	selector.add(client, net6::socket::INCOMING | net6::socket::OUTGOING | net6::socket::ERROR);

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
	net6::ipv4_address client_addr = net6::ipv4_address::create_from_hostname("localhost", port);
	net6::tcp_client_socket client(client_addr);

	client.read_event().connect(sigc::ptr_fun(&on_client_io) );
	client.write_event().connect(sigc::ptr_fun(&on_client_io) );
	client.error_event().connect(sigc::ptr_fun(&on_client_io) );

	selector.add(client, net6::socket::INCOMING | net6::socket::OUTGOING | net6::socket::ERROR);

	while(!quit)
		selector.select();
	
	return 0;
}

