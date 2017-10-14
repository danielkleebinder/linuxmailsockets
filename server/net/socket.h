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
class socket {
public:
	socket(int handler);
	socket(std::string host, int port);
	virtual ~socket();

	std::string get_host();
	int get_port();

	void close();

	stream get_stream();

protected:
	std::string _host;
	int _port;

private:
	int socket_handler;
};

#endif // SOCKET_H
