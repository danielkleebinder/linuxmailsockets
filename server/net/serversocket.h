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
	class serversocket {
	public:
		serversocket(int port);
		virtual ~serversocket();

		int get_port();

		void close();

		socket accept();

	protected:
		int _port;

	private:
		int socket_handler;
	};
}

#endif // SERVERSOCKET_H
