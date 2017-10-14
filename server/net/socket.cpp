/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 10.10.2017
 *
 * (c) All rights reserved
 */


#include "socket.h"
#include "stream.h"

#include <stdio.h>
#include <string>
#include <cstring>
#include <stdexcept>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>



net::ssocket::ssocket(int handler)
	: socket_handler(handler) {
	_stream = new stream(handler);
}

net::ssocket::ssocket(std::string host, int port)
	: _host(host), _port(port) {

	// Create socket
	socket_handler = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_handler == -1) {
		throw std::runtime_error("Could not create socket");
	}

	// Create socket connection
	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	inet_aton(host.c_str(), &address.sin_addr);

	// Try to connect to the specified host
	if (connect(socket_handler, (struct sockaddr*) &address, sizeof(address)) != 0) {
		throw std::runtime_error("Connection error, host not available");
	}

	// Create stream
	_stream = new stream(socket_handler);
}


net::ssocket::~ssocket() {
	delete _stream;
}


std::string net::ssocket::get_host() {
	return _host;
}

int net::ssocket::get_port() {
	return _port;
}

int net::ssocket::get_handler_id() {
	return socket_handler;
}

void net::ssocket::close_socket() {
	close(socket_handler);
	socket_handler = -1;
}

stream& net::ssocket::get_stream() {
	return *_stream;
}

