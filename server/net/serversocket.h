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
class serversocket {
public:
	serversocket(int handler);
	serversocket(std::string host, int port);
	virtual ~serversocket();

	std::string get_host();
	int get_port();

	void close();

	socket accept();

protected:
	std::string _host;
	int _port;

private:
	int socket_handler;
};

#endif // SOCKET_H
