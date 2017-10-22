/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 08.10.2017
 *
 * (c) All rights reserved
 */


#include "../user.h"
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

	int failed_login_attempts = 0;
	user current_user;
	do {
		// Read line by line using new streaming and socket API.
		// Rethrow any uncaught exceptions into the main procedure.
		line = socket_stream.sreadline();
	
		// Output debug
		if (debug) {
			std::cout << "(DM) Read line: " << line << std::endl;
		}

		// Check if line is null or empty
		if (line.empty()) {
			continue;
		}

		// Login user before any other protocol can be executed
		if (line == "LOGIN") {
			current_user = login();
			if (!current_user.is_logged_in()) {
				failed_login_attempts++;
			} else {
				failed_login_attempts = 0;
			}
		}

		// Check if user is authenticated
		if (!current_user.is_logged_in()) {
			continue;
		}

		// Start correct mail protocol
		run_smtp_protocols(current_user, line);
	} while (line != "quit");

	// Closing the socket
	quit(current_user);
	std::cout << "Closing socket ID-" << con_sock.get_handler_id() << std::endl;
	con_sock.close_socket();
}


void smtpservice::run_smtp_protocols(user& usr, std::string line) {
	if (line == "SEND") {
		send(usr);
	}

	if (line == "LIST") {
		list(usr);
	}

	if (line == "READ") {
		read(usr);
	}

	if (line == "DEL") {
		del(usr);
	}
}


user smtpservice::login() {
	stream in = socket.get_stream();

	// Create user and try logging in
	user usr;
	try {
		usr.set_username(in.sreadline());
		usr.set_password(in.sreadline());

		if (debug) {
			std::cout << "(DM) LOGIN Protocol: " << usr.get_username() << std::endl;
			std::cout << "(DM) LOGIN Protocol: PW: *****" << std::endl;
			std::cout << "(DM) LOGIN Protocol: " << usr.is_fhtw_user() << std::endl;
		}

		// TODO: Run LDAP login procedure here
		usr.set_logged_in(usr.is_fhtw_user());

		// Send "OK" if successfully logged in
		// Send "ERR" if the user failed to log in
		if (usr.is_logged_in()) {
			in.swrite("OK\n");
		} else {
			try_send_error(in);
		}

		return usr;
	} catch (std::exception& ex) {
		// An error occurred
		std::cout << ex.what() << std::endl;
		try_send_error(in);
	}

	return usr;
}


void smtpservice::send(user& usr) {
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
		try_send_error(in);
		return;
	}

	// Everything is fine
	in.swrite("OK\n");
}


void smtpservice::list(user& usr) {
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
		try_send_error(in);
	}
}


void smtpservice::read(user& usr) {
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
		try_send_error(in);
	}
}


void smtpservice::del(user& usr) {
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
		try_send_error(in);
	}
}


void smtpservice::quit(user& usr) {
	std::cout << "User quits the mail server!" << std::endl;
	std::stringstream ss;
	ss << "Disconnected from the server!\n";
	try {
		socket.get_stream().swrite(ss.str());
	} catch (std::exception& ex) {
		if (debug) {
			std::cout << "(DM) Socket already closed: " << ex.what() << std::endl;
		}
	}
}


void smtpservice::try_send_error(stream& in) {
	// Try to send the error code, but catch the exception which may
	// occur if the socket has been closed by the client side.
	try {
		in.swrite("ERR\n");
	} catch (std::exception& ex) {
		std::cout << ex.what() << std::endl;
	}
}

