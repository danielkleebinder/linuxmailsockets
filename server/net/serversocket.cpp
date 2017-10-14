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
#include <stdexcept>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>



net::serversocket::serversocket(int port)
	: _port(port) {

	// Create server socket handler
	socket_handler = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_handler == -1) {
		throw "Could not create server socket";
	}

	// Create address structure
	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	// Bind and listen to the max amount of connections of 5
	if (bind(socket_handler, (struct sockaddr*) &address, sizeof(address)) != 0) {
		throw "Could not bind the address to the server socket";
	}
	listen(socket_handler, 5);
}


net::serversocket::~serversocket() {
	if (socket_handler == -1) {
		return;
	}
	close();
}


int net::serversocket::get_port() {
	return _port;
}

void net::serversocket::close() {
	close(socket_handler);
	socket_handler = -1;
}

net::socket net::serversocket::accept() {
	socklen_t addrlen = sizeof(struct sockaddr_in);
	struct sockaddr_in client_address;
	int connection_socket = accept(socket_handler, (struct sockaddr*) &client_address, &addrlen);
	return socket(connection_socket);
}

