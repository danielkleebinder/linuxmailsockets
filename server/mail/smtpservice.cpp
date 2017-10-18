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
	: socket(socket), mps(mps), debug(false) {}

smtpservice::~smtpservice() {}


void smtpservice::start_forked_service() {
	std::thread con_thread(&smtpservice::run_protocol, this, std::ref(socket));
	con_thread.detach();
}


void smtpservice::start_service() {
	run_protocol(std::ref(socket));
}


void smtpservice::set_debug_mode(bool debug) {
	this->debug = debug;
}


bool smtpservice::get_debug_mode() {
	return debug;
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

		// Output debug
		if (debug) {
			std::cout << "(DM) Read line: " << line << std::endl;
		}
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

		if (debug) {
			std::cout << "(DM) SEND Protocol: " << mail.get_sender() << std::endl;
			std::cout << "(DM) SEND Protocol: " << mail.get_receiver() << std::endl;
			std::cout << "(DM) SEND Protocol: " << mail.get_subject() << std::endl;
		}
	
		// Read message until the char sequence "\n.\n" occurs
		std::stringstream message;
		std::string msg_ending = "\n.\n";
		std::string final_msg;
		while (true) {
			message << in.sreadline();
			message << '\n';
			final_msg = message.str();

			// Check if it is even possible to quit the message yet
			if (final_msg.length() < msg_ending.length()) {
				continue;
			}

			// Check if the message terminated successfully
			if (final_msg.substr(final_msg.length() - msg_ending.length()) == msg_ending) {
				break;
			}
		}
		mail.set_message(final_msg);
		mps.save_mail(mail);
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

		if (debug) {
			std::cout << "(DM) LIST Protocol: " << username << std::endl;
		}

		// List total amount of messages
		std::stringstream ss;
		ss << mails.size() << '\n';
		in.swrite(ss.str());

		// List all messages with numbers
		for (email current : mails) {
			ss.str(std::string());
			ss << current.get_subject() << '\n';
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

		if (debug) {
			std::cout << "(DM) READ Protocol: " << username << std::endl;
			std::cout << "(DM) READ Protocol: " << msg_num << std::endl;
		}

		// Load mail
		email mail = mps.load_mail(username, msg_num);

		// Nothing has thrown an exception, everything is fine so far
		in.swrite("OK\n");

		// Send E-Mail message
		std::stringstream ss;
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

		if (debug) {
			std::cout << "(DM) DEL Protocol: " << username << std::endl;
			std::cout << "(DM) DEL Protocol: " << msg_num << std::endl;
		}

		// Delete mail
		if (mps.delete_mail(username, msg_num)) {
			in.swrite("OK\n");
		} else {
			in.swrite("ERR\n");
		}
	} catch(std::exception& ex) {
		std::cout << ex.what() << std::endl;
		in.swrite("ERR\n");
	}
}


void smtpservice::quit() {
	std::cout << "User quits the mail server!" << std::endl;
	std::stringstream ss;
	ss << "Disconnected from the server!\n";
	socket.get_stream().swrite(ss.str());
}
