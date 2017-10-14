/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 08.10.2017
 *
 * (c) All rights reserved
 */


#include "../net/socket.h"
#include "smtpservice.h"

#include <string>
#include <iostream>
#include <thread>



smtpservice::smtpservice(net::ssocket& socket)
	: socket(socket) {}

smtpservice::~smtpservice() {}


void smtpservice::start_forked_service() {
	std::thread con_thread(&smtpservice::run_protocol, this, std::ref(socket));
	con_thread.detach();
}

void smtpservice::start_service() {
	run_protocol(std::ref(socket));
}

/**
 * Handles the connection from a client.
 *
 * @param con_sock Connected client socket.
 */
void smtpservice::run_protocol(net::ssocket& con_sock) {
	stream socket_stream = con_sock.get_stream();
	std::string line;
	do {
		// Read line by line using new streaming and socket API.
		// Rethrow any uncaught exceptions into the main procedure.
		line = socket_stream.sreadline();

		// Check if line is null or empty
		if (line.empty()) {
			continue;
		}

		// Start correct mail protocol
		if (line == "SEND") {
			send();
		}

		if (line == "LIST") {
			list();
		}

		if (line == "READ") {
			read();
		}

		if (line == "DEL") {
			del();
		}

		// Print received message
		std::cout << "Message Received: " << line << std::endl;
	} while (line != "quit");

	// Closing the socket
	quit();
	std::cout << "Closing socket ID-" << con_sock.get_handler_id() << std::endl;
	con_sock.close_socket();
}


void smtpservice::send() {
	socket.get_stream().swrite("OK\n");
}

void smtpservice::list() {
	std::cout << "LIST PROTOCOL" << std::endl;
	socket.get_stream().swrite("List Protocol started\n");
}

void smtpservice::read() {
	std::cout << "READ PROTOCOL" << std::endl;
	socket.get_stream().swrite("Read Protocol started\n");
}

void smtpservice::del() {
	std::cout << "DEL PROTOCOL" << std::endl;
	socket.get_stream().swrite("Del Protocl started\n");
}

void smtpservice::quit() {
	std::cout << "QUIT PROTOCOL" << std::endl;
	socket.get_stream().swrite("Quit Protocol started\n");
}
