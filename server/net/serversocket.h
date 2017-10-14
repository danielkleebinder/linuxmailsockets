#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

#include "socket.h"
#include <string>

/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 12.10.2017
 *
 * (c) All rights reserved
 */
namespace net {
	class sserversocket {
	public:
		sserversocket(int port);
		virtual ~sserversocket();

		int get_port();

		void close_socket();

		net::ssocket accept_connection();

	protected:
		int _port;

	private:
		int socket_handler;
	};
}

#endif // SERVERSOCKET_H
