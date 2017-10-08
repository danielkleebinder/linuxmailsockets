/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 08.10.2017
 *
 * (c) All rights reserved
 */


#include "smtphandler.h"
#include "netutils.h"

#include <string>
#include <iostream>



smtphandler::smtphandler(int socket)
	: socket(socket) {}

smtphandler::~smtphandler() {}



void smtphandler::send() {
	netutils::writetext(socket, "OK\n");
}

void smtphandler::list() {
	std::cout << "LIST PROTOCOL" << std::endl;
	netutils::writetext(socket, "List Protocol started\n");
}

void smtphandler::read() {
	std::cout << "READ PROTOCOL" << std::endl;
	netutils::writetext(socket, "Read Protocol started\n");
}

void smtphandler::del() {
	std::cout << "DEL PROTOCOL" << std::endl;
	netutils::writetext(socket, "Del Protocol started\n");
}

void smtphandler::quit() {
	std::cout << "QUIT PROTOCOL" << std::endl;
	netutils::writetext(socket, "Quit Protocol started\n");
}
