WARNINGS = -ansi -Wall -Wfloat-equal -Wpointer-arith -Werror -Wnon-template-friend -Woverloaded-virtual -Wsign-promo -Wpmf-conversions -Wsynth -Wcast-qual
COMP_FLAGS = $(CXXFLAGS) $(WARNINGS) `pkg-config --cflags sigc++-2.0` -DPIC -fPIC -DBUILDING_DLL -DCOMPILING_NET6 -I.
LINK_FLAGS = $(LIBS) `pkg-config --libs sigc++-2.0`

OBJ_ERROR = obj/error.o
OBJ_MAIN = obj/main.o
OBJ_ADDRESS = obj/address.o
OBJ_SOCKET = obj/socket.o
OBJ_SELECT = obj/select.o
OBJ_PACKET = obj/packet.o
OBJ_CONNECTION = obj/connection.o
OBJ_PEER = obj/peer.o
OBJ_SERVER = obj/server.o
OBJ_CLIENT = obj/client.o

OBJS = $(OBJ_ERROR) $(OBJ_MAIN) $(OBJ_ADDRESS) $(OBJ_SOCKET) $(OBJ_SELECT) $(OBJ_PACKET) $(OBJ_CONNECTION) $(OBJ_PEER) $(OBJ_SERVER) $(OBJ_CLIENT)

libnet6.so: $(OBJS)
	g++ -shared -o libnet6.so $(OBJS) $(LINK_FLAGS)

$(OBJ_ERROR): src/error.cpp inc/error.hpp
	g++ -c src/error.cpp -o $(OBJ_ERROR) $(COMP_FLAGS)
$(OBJ_MAIN): inc/error.hpp src/main.cpp inc/main.hpp
	g++ -c src/main.cpp -o $(OBJ_MAIN) $(COMP_FLAGS)
$(OBJ_ADDRESS): inc/error.hpp src/address.cpp inc/address.hpp
	g++ -c src/address.cpp -o $(OBJ_ADDRESS) $(COMP_FLAGS)
$(OBJ_SOCKET): inc/error.hpp inc/address.hpp src/socket.cpp inc/socket.hpp
	g++ -c src/socket.cpp -o $(OBJ_SOCKET) $(COMP_FLAGS)
$(OBJ_SELECT): inc/error.hpp inc/socket.hpp src/select.cpp inc/select.hpp
	g++ -c src/select.cpp -o $(OBJ_SELECT) $(COMP_FLAGS)
$(OBJ_PACKET): src/packet.cpp inc/packet.hpp
	g++ -c src/packet.cpp -o $(OBJ_PACKET) $(COMP_FLAGS)
$(OBJ_CONNECTION): inc/socket.hpp inc/packet.hpp src/connection.cpp inc/connection.hpp
	g++ -c src/connection.cpp -o $(OBJ_CONNECTION) $(COMP_FLAGS)
$(OBJ_PEER): src/peer.cpp inc/peer.hpp
	g++ -c src/peer.cpp -o $(OBJ_PEER) $(COMP_FLAGS)
$(OBJ_SERVER): inc/connection.hpp inc/peer.hpp src/server.cpp inc/server.hpp
	g++ -c src/server.cpp -o $(OBJ_SERVER) $(COMP_FLAGS)
$(OBJ_CLIENT): inc/connection.hpp inc/peer.hpp src/client.cpp inc/client.hpp
	g++ -c src/client.cpp -o $(OBJ_CLIENT) $(COMP_FLAGS)

install: libnet6.so
	cp libnet6.so /usr/local/lib
	mkdir -p /usr/local/include/net6
	cp inc/* /usr/local/include/net6

uninstall:
	rm -Rf /usr/local/include/net6
	rm -f /usr/local/lib/libnet6.so

clean:
	make -C test clean
	rm -f obj/*.o
	rm -f libnet6.so

test: libnet6.so
	make -C test
