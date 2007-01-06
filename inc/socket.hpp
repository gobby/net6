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

#ifndef _NET6_SOCKET_HPP_
#define _NET6_SOCKET_HPP_

#include <gnutls/gnutls.h>

#include <fcntl.h>
#include <memory>
#include <sigc++/signal.h>

#include "error.hpp"
#include "enum_ops.hpp"
#include "non_copyable.hpp"
#include "address.hpp"

namespace net6
{

// Newer versions of GNUTLS use types suffixed with _t.
typedef gnutls_session gnutls_session_t;
typedef gnutls_anon_client_credentials gnutls_anon_client_credentials_t;
typedef gnutls_anon_server_credentials gnutls_anon_server_credentials_t;
typedef gnutls_transport_ptr gnutls_transport_ptr_t;

enum io_condition
{
	IO_NONE     = 0x00,
	IO_INCOMING = 0x01,
	IO_OUTGOING = 0x02,
	IO_ERROR    = 0x04
};

NET6_DEFINE_ENUM_OPS(io_condition)

/** Abstract socket class.
 */
class socket: private non_copyable
{
public:
	typedef sigc::signal<void, io_condition> signal_io_type;

#ifdef WIN32
	typedef SOCKET socket_type;
#else
	typedef int socket_type;
#endif
	typedef size_t size_type;

	/** @brief Closes the socket.
	 */
	~socket();

	/** Signal which will be emitted if somehting occures with the socket.
	 */
	signal_io_type io_event() const { return signal_io; }

	/** Provides access to the underlaying C socket object.
	 */
	socket_type cobj() { return sock; }
	const socket_type cobj() const { return sock; }

	/** Invalidates the current socket object, which causes all calls
	 * to fail.
	 */
	void invalidate();

protected:
	socket(int domain, int type, int protocol);
	socket(socket_type c_object);

private:
	socket_type sock;
	signal_io_type signal_io;
};

/** Abstract TCP socket class.
 */
class tcp_socket: public socket
{
protected:
	tcp_socket(const address& addr);
	tcp_socket(socket_type c_object);
};

/** TCP connection socket.
 */

class tcp_client_socket: public tcp_socket
{
public:
	/** Creates a new tcp socket and connects to the address addr.
	 */
	tcp_client_socket(const address& addr);

	/** Wraps a C socket object. Note that the tcp_client_socket owns
	 * the C object.
	 */
	tcp_client_socket(socket_type c_object);
	virtual ~tcp_client_socket();

	/** Sends an amount of data through the socket. Note that the call
	 * may block if you did not select on a socket::OUT event.
	 * @return The amount of data sent.
	 */
	virtual size_type send(const void* buf, size_type len) const;

	/** Receives an amount of data from the socket. Note that the call
	 * may block if no data is available.
	 * @return The amount of data read.
	 */
	virtual size_type recv(void* buf, size_type len) const;
};

class tcp_encrypted_socket_base: public tcp_client_socket
{
public:
	virtual ~tcp_encrypted_socket_base();

	/** @brief Initiates a TLS handshake.
	 *
	 * Returns TRUE when the handshake has been completed and FALSE when
	 * further data needs to be transmitted. You may then select and call
	 * this function again when data is availabe to send and/or receive
	 * (see tcp_encrypted_socket_base::get_dir()).
	 *
	 * TODO: Possibility to make this blocking
	 */
	bool handshake();

	/** Returns <em>true</em> when GNUTLS tried to send data, but failed.
	 * and <em>false</em> when GNUTLS tried to receive.
	 */
	bool get_dir() const;

	/** @brief Returns the amount of bytes remaining in the GnuTLS buffers.
	 *
	 * If a socket is selected for IO_INCOMING, the selector would not
	 * return for this socket even if there is still data to be read when
	 * GnuTLS already read that data and keeps it in its internal buffer.
	 */
	size_type get_pending() const;

	/** @brief Tries to send <em>len</em> bytes of data starting at 
	 * <em>buf</em>.
	 *
	 * The function returns the amount of bytes actually sent that may
	 * be less than <em>len</em>.
	 *
	 * A handshake must have been performed before using this function.
	 */
	virtual size_type send(const void* buf, size_type len) const;

	/** @brief Tries to read <em>len</em> bytes of data into the buffer
	 * starting at <em>buf</em>.
	 *
	 * The function returns the amount of bytes actually read that may
	 * be less than <em>len</em>.
	 *
	 * A handshake must have been performed before using this function.
	 */
	virtual size_type recv(void* buf, size_type len) const;

protected:
	/** Ownership of session is given to tcp_encrypted_socket_base.
	 */
	tcp_encrypted_socket_base(socket_type cobj, gnutls_session_t sess);

	template<
		typename buffer_type,
		ssize_t(*iofunc)(gnutls_session_t, buffer_type, size_t)
	> size_type io_impl(buffer_type buf, size_type len) const;

	enum handshake_state {
		DEFAULT,
		HANDSHAKING,
		HANDSHAKED
	};

	gnutls_session_t session;
	handshake_state state;
	bool was_blocking;
};

class tcp_encrypted_socket_client: public tcp_encrypted_socket_base
{
public:
	tcp_encrypted_socket_client(tcp_client_socket& sock);
	virtual ~tcp_encrypted_socket_client();

private:
	//static gnutls_session_t create_session(int fd);

	typedef gnutls_anon_client_credentials_t credentials_type;
	credentials_type anoncred;
};

class tcp_encrypted_socket_server: public tcp_encrypted_socket_base
{
public:
	tcp_encrypted_socket_server(tcp_client_socket& sock);
	virtual ~tcp_encrypted_socket_server();

private:
	typedef gnutls_anon_server_credentials_t credentials_type;
	credentials_type anoncred;

	gnutls_dh_params_t dh_params;
};

#if 0
/** Encrypted TCP connection socket.
 */
template<typename Info>
class basic_tcp_encrypted_socket: public tcp_encrypted_socket_base
{
public:
	basic_tcp_encrypted_socket(tcp_client_socket& sock);
	virtual ~basic_tcp_encrypted_socket();

private:
	static std::auto_ptr<Info> create_session(int fd);

	std::auto_ptr<Info> info;
//	Info my_info; //std::auto_ptr<Info> info;
	//typename Info::credentials_type anoncred;

	template<
		typename buffer_type,
		ssize_t(*iofunc)(gnutls_session_t, buffer_type, size_t)
	> size_type io_impl(buffer_type buf, size_type len) const;
};

typedef basic_tcp_encrypted_socket<server_info> tcp_encrypted_socket_server;
typedef basic_tcp_encrypted_socket<client_info> tcp_encrypted_socket_client;
#endif

/** TCP server socket
*/

class tcp_server_socket: public tcp_socket
{
public:
	/** Opens a new TCP server socket bound to <em>bind_addr</em>.
	 */
	tcp_server_socket(const address& bind_addr);

	/** Wraps a C socket object. Note that the tcp_server_socket owns the
	 * C object.
	 */
	tcp_server_socket(socket_type c_object);

	/** Accepts a connection from this server socket. Note that the call
	 * blocks until a connection comes available. Selecting on socket::IN
	 * indicates a connection waiting for acception.
	 * @return A tcp_client_socket to communicate with the remote host.
	 */
	std::auto_ptr<tcp_client_socket> accept() const;

	/** Accepts a new connection and stores the address of the remote host
	 * in <em>from</em>.
	 */
	std::auto_ptr<tcp_client_socket> accept(address& from) const;
};

/** UDP socket.
 */
class udp_socket: public socket
{
public:
	/** Creates a new UDP socket bound to <em>bind_addr</em>.
	 */
	udp_socket(const address& bind_addr);

	/** Wraps a C UDP socket object.
	 */
	udp_socket(socket_type c_object);

	/** Sets the target of this UDP socket. This target is the address to
	 * which datagrams are sent by default and the only address from which
	 * datagrams are received.
	 */
	void set_target(const address& addr);

	/** Resets the target of the UDP socket.
	 */
	void reset_target();

	/** Sends an amount of data to the target of the UDP socket.
	 * @return The amount of data actually sent.
	 */
	size_type send(const void* buf,
	               size_type len) const;

	/** Sends an amount of data to a specified address.
	 * @return The amount of data actually sent.
	 */
	size_type send(const void* bud,
	               size_type len,
	               const address& to) const;

	/** Receives some data from the socket. Note that the call may block
	 * until data becomes available.
	 * @return The amount of data actually read.
	 */
	size_type recv(void* buf,
	               size_type len) const;

	/** Receives some data from the socket and stores the source
	 * address into <em>from</em>. Note that the call may block until
	 * data becomes available for reading.
	 * @return The amount of data actually read.
	 */
	size_type recv(void* buf,
	               size_type len,
	               address& from) const;
};

template<
	typename buffer_type,
	ssize_t(*func)(gnutls_session_t, buffer_type, size_t)
> typename tcp_encrypted_socket_base::size_type
tcp_encrypted_socket_base::io_impl(buffer_type buf, size_type len) const
{
	if(state == HANDSHAKING)
	{
		throw std::logic_error(
			"net6::tcp_encrypted_socket_base::io_impl:\n"
			"IO tried while handshaking"
		);
	}

	if(state == DEFAULT)
	{
		throw std::logic_error(
			"net6::tcp_encrypted_socket_base::io_impl:\n"
			"Handshake not yet performed"
		);
	}

	ssize_t ret = func(session, buf, len);
	if(ret == GNUTLS_E_AGAIN || ret == GNUTLS_E_INTERRUPTED)
	{
		func(session, NULL, 0);
	}

	if(ret < 0)
		throw net6::error(net6::error::GNUTLS, ret);

	return ret;
}

} // namespace net6

#endif // _NET6_SOCKET_HPP_
