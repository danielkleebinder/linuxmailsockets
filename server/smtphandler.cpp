/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 08.10.2017
 *
 * (c) All rights reserved
 */


#include "net/socket.h"
#include "smtphandler.h"

#include <string>
#include <iostream>



smtphandler::smtphandler(net::socket& socket)
	: socket(socket) {}

smtphandler::~smtphandler() {}



void smtphandler::send() {
	socket.get_stream().swrite("OK\n");
}

void smtphandler::list() {
	std::cout << "LIST PROTOCOL" << std::endl;
	socket.get_stream().swrite("List Protocol started\n");
}

void smtphandler::read() {
	std::cout << "READ PROTOCOL" << std::endl;
	socket.get_stream().swrite("Read Protocol started\n");
}

void smtphandler::del() {
	std::cout << "DEL PROTOCOL" << std::endl;
	socket.get_stream().swrite("Del Protocl started\n");
}

void smtphandler::quit() {
	std::cout << "QUIT PROTOCOL" << std::endl;
	socket.get_stream().swrite("Quit Protocol started\n");
}
