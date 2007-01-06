
#include <iostream>
#include <sigc++/bind.h>

#include <net6/main.hpp>
#include <net6/packet.hpp>
#include <net6/server.hpp>
#include <net6/client.hpp>

const int port = 1364;
bool quit = false;

int client_main(int argc, char* argv[]);

void on_server_join(net6::server::peer& peer, net6::server& server)
{
	std::cout << peer.get_address().get_name() << " connected" << std::endl;
}

void on_server_login(net6::server::peer& peer, const net6::packet& pack, net6::server& server)
{
	std::cout << peer.get_address().get_name() << " logged in as " << peer.get_name() << std::endl;
}
	
void on_server_part(net6::server::peer& peer, net6::server& server)
{
	std::cout << peer.get_name() << " disconnected" << std::endl;
}

void on_server_data(const net6::packet& pack, net6::server::peer& peer, net6::server& server)
{
	if(pack.get_command() == "message")
	{
		net6::packet fwd_pack("message");
		fwd_pack << static_cast<int>(peer.get_id() ) << pack.get_param(0).as_string();
		server.send(fwd_pack);
	}
}

bool on_server_auth(net6::server::peer& peer, const net6::packet& pack, std::string& reason)
{
	if(pack.get_param(0).as_string() == "foo")
	{
		reason = "foo ist nicht erlaubt!";
		return false;
	}
	else
		return true;
}

void on_client_join(net6::client::peer& peer, const net6::packet& pack, net6::client& client)
{
	std::cout << peer.get_name() << " joined" << std::endl;
}

void on_client_part(net6::client::peer& peer, net6::client& client)
{
	std::cout << peer.get_name() << " left" << std::endl;
}

void on_client_data(const net6::packet& pack, net6::client& client)
{
	if(pack.get_command() == "message")
	{
		int id = pack.get_param(0).as_int();
		std::string msg = pack.get_param(1).as_string();

		net6::client::peer* from = client.find(id);
		std::cout << "<" << from->get_name() << "> " << msg << std::endl;
	}
}

void on_client_close(net6::client& client)
{
	std::cout << "Connection lost" << std::endl;
	quit = 1;
}

void on_client_login_failed(const std::string& reason, net6::client& client)
{
	std::cout << reason << std::endl;
	quit = 1;
}

int main(int argc, char* argv[]) try
{
	net6::main kit;

	if(argc > 1)
		return client_main(argc, argv);

	net6::server server(port, false);

	server.join_event().connect(sigc::bind(sigc::ptr_fun(&on_server_join), sigc::ref(server)) );
	server.pre_login_event().connect(sigc::bind(sigc::ptr_fun(&on_server_login), sigc::ref(server)) );
	server.login_auth_event().connect(sigc::ptr_fun(&on_server_auth) );
	server.part_event().connect(sigc::bind(sigc::ptr_fun(&on_server_part), sigc::ref(server)) );
	server.data_event().connect(sigc::bind(sigc::ptr_fun(&on_server_data), sigc::ref(server)) );
	
	while(!quit)
		server.select();

	return 0;
}
catch(std::exception& e)
{
	std::cerr << e.what() << std::endl;
}

int client_main(int argc, char* argv[])
{
	net6::ipv4_address addr(net6::ipv4_address::create_from_hostname("localhost", port) );

	net6::client client(addr);
	client.login(argv[1]);

	net6::packet pack("message");
	pack << "foobar";
	client.send(pack);

	client.join_event().connect(sigc::bind(sigc::ptr_fun(&on_client_join), sigc::ref(client))  );
	client.part_event().connect(sigc::bind(sigc::ptr_fun(&on_client_part), sigc::ref(client)) );
	client.data_event().connect(sigc::bind(sigc::ptr_fun(&on_client_data), sigc::ref(client)) );
	client.close_event().connect(sigc::bind(sigc::ptr_fun(&on_client_close), sigc::ref(client)) );
	client.login_failed_event().connect(sigc::bind(sigc::ptr_fun(&on_client_login_failed), sigc::ref(client)) );
	
	while(!quit)
		client.select();

	return 0;
}

