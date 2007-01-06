/* net6 - Library providing IPv4/IPv6 network access
 * Copyright (C) 2005, 2006 Armin Burgmeier / 0x539 dev group
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

#ifndef _NET6_ENCRYPT_HPP_
#define _NET6_ENCRYPT_HPP_

#include <memory>
#include <gnutls/gnutls.h>
#include "socket.hpp"

//#include <fcntl.h>

namespace net6
{

// Newer versions of GNUTLS use types suffixed with _t.
typedef gnutls_session gnutls_session_t;
typedef gnutls_anon_client_credentials gnutls_anon_client_credentials_t;
typedef gnutls_anon_server_credentials gnutls_anon_server_credentials_t;
typedef gnutls_transport_ptr gnutls_transport_ptr_t;
typedef gnutls_dh_params gnutls_dh_params_t;
typedef gnutls_connection_end gnutls_connection_end_t;

class dh_params: private net6::non_copyable
{
public:
	/** @brief Generates new params.
	 */
	dh_params();

	/** @brief Takes ownership of given dh params.
	 */
	dh_params(gnutls_dh_params_t initial_params);

	~dh_params();

	gnutls_dh_params_t cobj();
	const gnutls_dh_params_t cobj() const;

protected:
	gnutls_dh_params_t params;
};

class tcp_encrypted_socket_base: public tcp_client_socket
{
public:
	enum handshake_state {
		DEFAULT,
		HANDSHAKING,
		HANDSHAKED
	};

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
	typedef gnutls_anon_client_credentials_t credentials_type;
	credentials_type anoncred;
};

class tcp_encrypted_socket_server: public tcp_encrypted_socket_base
{
public:
	/** @brief Constructor creating its own dh_params.
	 */
	tcp_encrypted_socket_server(tcp_client_socket& sock);

	/** @brief Constructor using the given dh_params.
	 */
	tcp_encrypted_socket_server(tcp_client_socket& sock, dh_params& params);
	virtual ~tcp_encrypted_socket_server();

private:
	typedef gnutls_anon_server_credentials_t credentials_type;
	credentials_type anoncred;

	std::auto_ptr<dh_params> own_params;
};

} // namespace net6

#endif // _NET6_ENCRYPT_HPP_
