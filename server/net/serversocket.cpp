/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 12.10.2017
 *
 * (c) All rights reserved
 */


#include "serversocket.h"
#include "socket.h"

#include <string>
#include <stdexcept>
#include <unistd.h>


serversocket::serversocket(int handler)
	: socket_handler(handler) {}

serversocket::serversocket(std::string host, int port)
	: _host(host), _port(port) {}


serversocket::~serversocket() {}


std::string serversocket::get_host() {
	return _host;
}

int serversocket::get_port() {
	return _port;
}

void serversocket::close() {
	close(socket_handler);
}

socket serversocket::accept() {
	return socket();
}

