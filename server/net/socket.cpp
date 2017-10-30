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



net::csocket::csocket(int handler)
	: socket_handler(handler) {
	_stream = new stream(handler);
}

net::csocket::csocket(std::string host, int port)
	: _host(host), _port(port) {}


net::csocket::~csocket() {
	delete _stream;
}


void net::csocket::set_host(std::string host) {
	if (socket_handler < 0) {
		throw std::runtime_error("Socket is already bound");
	}
	_host = host;
}


std::string net::csocket::get_host() {
	return _host;
}


void net::csocket::set_port(int port) {
	if (socket_handler < 0) {
		throw std::runtime_error("Socket is already bound");
	}
	_port = port;
}


int net::csocket::get_port() {
	return _port;
}


int net::csocket::get_handler_id() {
	return socket_handler;
}


void net::csocket::bind() {
	// Create socket
	socket_handler = ::socket(AF_INET, SOCK_STREAM, 0);
	if (socket_handler == -1) {
		throw std::runtime_error("Could not create socket");
	}

	// Create socket connection
	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(_port);
	inet_aton(_host.c_str(), &address.sin_addr);

	// Try to connect to the specified host
	if (connect(socket_handler, (struct sockaddr*) &address, sizeof(address)) != 0) {
		throw std::runtime_error("Connection error, host not available");
	}

	// Create stream
	_stream = new stream(socket_handler);

}


void net::csocket::close() {
	::close(socket_handler);
	socket_handler = -1;
}


stream& net::csocket::get_stream() {
	if (socket_handler < 0) {
		throw std::runtime_error("Unbound socket, no stream available yet");
	}
	return *_stream;
}

