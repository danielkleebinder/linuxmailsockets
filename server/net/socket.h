#ifndef SOCKET_H
#define SOCKET_H

#include "stream.h"
#include <string>

/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 10.10.2017
 *
 * (c) All rights reserved
 */
namespace net {
	class ssocket {
	public:
		ssocket(int handler);
		ssocket(std::string host, int port);
		virtual ~ssocket();

		std::string get_host();
		int get_port();
		int get_handler_id();

		void close_socket();

		stream& get_stream();

	protected:
		std::string _host;
		int _port;
		stream* _stream;

	private:
		int socket_handler;
	};
}

#endif // SOCKET_H