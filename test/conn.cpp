
#include <iostream>
#include <sigc++/bind.h>

#include <net6/main.hpp>
#include <net6/address.hpp>
#include <net6/socket.hpp>
#include <net6/packet.hpp>
#include <net6/connection.hpp>
#include <net6/select.hpp>

const int port = 2349;
bool quit = false;
net6::selector selector;

int client_main(int argc, char* argv[]);

void on_server_recv(const net6::packet& pack)
{
	std::cout << "Received " << pack.get_command() << std::endl;
	std::cout << pack.get_param(0).as<std::string>() << std::endl;
}

void on_server_send(const net6::connection<net6::selector>& conn)
{
	std::cout << "Sent packet" << std::endl;

	selector.set(
		conn.get_socket(),
		selector.get(conn.get_socket()) & ~net6::IO_OUTGOING
	);
}

void on_server_close()
{
	std::cout << "Connection lost" << std::endl;
	quit = true;
}

void on_client_recv(const net6::packet& pack)
{
	std::cout << "Received " << pack.get_command() << std::endl;
	std::cout << pack.get_param(0).as<std::string>() << std::endl;
}

void on_client_send(const net6::connection<net6::selector>& conn)
{
	std::cout << "Sent packet" << std::endl;

	selector.set(
		conn.get_socket(),
		selector.get(conn.get_socket()) & ~net6::IO_OUTGOING
	);
}

void on_client_close()
{
	std::cout << "Connection lost" << std::endl;
	quit = true;
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

	net6::connection<net6::selector> conn(selector);
	conn.assign(client, client_addr);

	std::cout << "Connection from "
	          << conn.get_remote_address().get_name()
	          << std::endl;

	conn.recv_event().connect(sigc::ptr_fun(&on_server_recv) );
	conn.send_event().connect(
		sigc::bind(sigc::ptr_fun(&on_server_send), sigc::ref(conn))
	);
	conn.close_event().connect(sigc::ptr_fun(&on_server_close) );

	selector.set(
		conn.get_socket(),
		net6::IO_INCOMING | net6::IO_OUTGOING | net6::IO_ERROR
	);

	net6::packet pack("message");
	pack << "Hallo, Welt!";
	conn.send(pack);

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

	net6::connection<net6::selector> conn(selector);
	conn.connect(client_addr);

	conn.recv_event().connect(sigc::ptr_fun(&on_client_recv) );
	conn.send_event().connect(
		sigc::bind(sigc::ptr_fun(&on_client_send), sigc::ref(conn))
	);
	conn.close_event().connect(sigc::ptr_fun(&on_client_close) );

	selector.set(
		conn.get_socket(),
		net6::IO_INCOMING | net6::IO_OUTGOING | net6::IO_ERROR
	);

	net6::packet pack("message");
	pack << "foobar";
	conn.send(pack);

	while(!quit)
		selector.select();
	
	return 0;
}

