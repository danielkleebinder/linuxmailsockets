/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 12.10.2017
 *
 * (c) All rights reserved
 */


#include "socket.h"
#include "serversocket.h"

#include <stdio.h>
#include <string>
#include <cstring>
#include <stdexcept>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>




net::serversocket::serversocket(int port) : _port(port) {}
net::serversocket::~serversocket() {}



void net::serversocket::set_port(int port) {
	if (socket_handler < 0) {
		throw std::runtime_error("Server socket is already bound");
	}
	_port = port;
}


int net::serversocket::get_port() {
	return _port;
}


void net::serversocket::bind() {
	// Create server socket handler
	socket_handler = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_handler == -1) {
		throw std::runtime_error("Could not create server socket");
	}
	// Create address structure
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(_port);

	// Bind and listen to the max amount of connections of 5
	if (::bind(socket_handler, (struct sockaddr*) &addr, sizeof(addr)) != 0) {
		throw std::runtime_error("Could not bind the address to the server socket, maybe try another port number!");
	}
	listen(socket_handler, 5);
}


void net::serversocket::close() {
	::close(socket_handler);
	socket_handler = -1;
}


/**
 * Blocks until a new client tries to connect.
 *
 * @return Accepted socket connection.
 */
net::csocket* net::serversocket::accept() {
	if (socket_handler < 0) {
		throw std::runtime_error("Server socket not bound");
	}

	// Wait for connection
	socklen_t addrlen = sizeof(struct sockaddr_in);
	struct sockaddr_in client_address;
	int connection_socket = ::accept(socket_handler, (struct sockaddr*) &client_address, &addrlen);

	// Create the socket, set all parameters and return the value
	net::csocket* result = new net::csocket(connection_socket);
	result->set_host(std::string("127.0.0.1"));
	result->set_address(convert_address(client_address));
	result->set_port(htons(client_address.sin_port));
	return result;
}


/**
 * Converts the given C address in structure into a C++ string object.
 *
 * @param ca Socket address in structure.
 * @return IP address as string.
 */
std::string net::serversocket::convert_address(struct sockaddr_in ca) {
	struct in_addr addr = ca.sin_addr;

	char str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &addr, str, INET_ADDRSTRLEN);

	return std::string(str);
}

