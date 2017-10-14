/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 10.10.2017
 *
 * (c) All rights reserved
 */


#include "socket.h"
#include "stream.h"

#include <string>
#include <stdexcept>
#include <unistd.h>


socket::socket(int handler)
	: socket_handler(handler) {
	_stream = new stream(handler);
}

socket::socket(std::string host, int port)
	: _host(host), _port(port) {}


socket::~socket() {
	delete _stream;
}


std::string socket::get_host() {
	return _host;
}

int socket::get_port() {
	return _port;
}

void socket::close() {
	close(socket_handler);
}

stream socket::get_stream() {
	return _stream;
}

