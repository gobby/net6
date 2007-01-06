
#include <iostream>
#include <sigc++/bind.h>

#include <net6/main.hpp>
#include <net6/select.hpp>
#include <net6/connection.hpp>

const int PORT = 35267;

void on_close(net6::selector& selector)
{
	std::cout << "Connection lost" << std::endl;
	selector.quit();
}

void exec(net6::connection<net6::selector>& conn, net6::selector& selector)
{
	conn.set_enable_keepalives(true);

	conn.close_event().connect(
		sigc::bind(
			sigc::ptr_fun(&on_close),
			sigc::ref(selector)
		)
	);

	selector.run();
}

int main(int argc, char* argv[]) try
{
	net6::main kit;

	net6::selector selector;

	if(argc == 1)
	{
		net6::ipv4_address serv_addr(PORT);
		net6::tcp_server_socket server(serv_addr);

		net6::ipv4_address client_addr;
		std::auto_ptr<net6::tcp_client_socket> client;
		client = server.accept(client_addr);

		net6::connection<net6::selector> conn(selector);
		conn.assign(client, client_addr);
		exec(conn, selector);
	}
	else
	{
		net6::ipv4_address client_addr =
			net6::ipv4_address::create_from_hostname(
				"localhost",
				PORT
			);

		net6::connection<net6::selector> conn(selector);
		conn.connect(client_addr);

		exec(conn, selector);
	}
}
catch(std::exception& e)
{
	std::cerr << e.what() << std::endl;
}
