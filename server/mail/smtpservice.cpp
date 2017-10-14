/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 08.10.2017
 *
 * (c) All rights reserved
 */


#include "../net/socket.h"
#include "smtpservice.h"
#include "mailpoolservice.h"

#include <string>
#include <iostream>
#include <thread>
#include <stdexcept>
#include <vector>
#include <sstream>


smtpservice::smtpservice(net::ssocket& socket, mailpoolservice& mps)
	: socket(socket), mps(mps) {}

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
	stream in = socket.get_stream();

	// Create E-Mail
	email mail;
	try {
		mail.set_sender(in.sreadline());
		mail.set_receiver(in.sreadline());
		mail.set_subject(in.sreadline());

		// Read message until the char sequence "\n.\n" occurs
		std::string message, line;
		while (true) {
			line = in.sreadline();
			message += line;
			message += '\n';
			if (message.substr(message.length() - 3) == "\n.\n") {
				break;
			}
		}
		mail.set_message(message);
	} catch (std::exception& ex) {
		// An error occurred
		std::cout << ex.what() << std::endl;
		in.swrite("ERR\n");
		return;
	}

	// Everything is fine
	in.swrite("OK\n");
}

void smtpservice::list() {
	stream in = socket.get_stream();

	try {
		std::string username = in.sreadline();
		std::vector<email> mails = mps.load_user_mails(username);

		// List total amount of messages
		std::stringstream ss;
		ss << "Number of available messages: ";
		ss << mails.size();
		ss << std::endl;
		in.swrite(ss.str());

		// List all messages with numbers
		int index = 0;
		for (email current : mails) {
			ss.str(std::string());
			ss << " " << (++index) << ".) ";
			ss << current.get_subject();
			ss << std::endl;
			in.swrite(ss.str());
		}
	} catch(std::exception& ex) {
		std::cout << ex.what() << std::endl;
		in.swrite("ERR\n");
		return;
	}
}

void smtpservice::read() {
	stream in = socket.get_stream();

	try {
		std::string username = in.sreadline();
		int msg_num = atoi(in.sreadline().c_str());

		// Load mail
		email mail = mps.load_mail(username, msg_num);

		// Nothing has thrown an exception, everything is fine so far
		std::stringstream ss;
		ss << "OK" << std::endl;
		in.swrite(ss.str());

		// Send E-Mail message
		ss.str(std::string());
		ss << mail.get_message();
		ss << std::endl;
		in.swrite(ss.str());
	} catch(std::exception& ex) {
		std::cout << ex.what() << std::endl;
		in.swrite("ERR\n");
		return;
	}
}

void smtpservice::del() {
	stream in = socket.get_stream();

	try {
		std::string username = in.sreadline();
		int msg_num = atoi(in.sreadline().c_str());

		// Delete mail
		mps.delete_mail(username, msg_num);

		// Nothing has thrown an exception, everything is fine
		std::stringstream ss;
		ss << "OK" << std::endl;
		in.swrite(ss.str());
	} catch(std::exception& ex) {
		std::cout << ex.what() << std::endl;
		in.swrite("ERR\n");
		return;
	}
}

void smtpservice::quit() {
	std::cout << "User quits the mail server!" << std::endl;
	std::stringstream ss;
	ss << "Disconnected from the server!" << std::endl;
	socket.get_stream().swrite(ss.str());
}