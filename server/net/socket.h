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
	class csocket {
	public:
		csocket(int handler);
		csocket(std::string host, int port);
		virtual ~csocket();

		void set_host(std::string host);
		std::string get_host();
		void set_port(int port);
		int get_port();
		int get_handler_id();

		void bind();
		void close();

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
