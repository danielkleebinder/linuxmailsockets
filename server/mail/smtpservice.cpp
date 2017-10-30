/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 08.10.2017
 *
 * (c) All rights reserved
 */


#include "../user.h"
#include "../raiilock.h"
#include "../net/socket.h"
#include "smtpservice.h"
#include "mailpoolservice.h"

#include <string>
#include <iostream>
#include <thread>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <mutex>
#include <map>


// Initialize static variables
std::map<std::string, int> smtpservice::login_attempts;
std::mutex smtpservice::login_attempts_mutex;


// SMTP Service constructor
smtpservice::smtpservice(net::csocket& socket, mailpoolservice& mps, loginsystem& ls)
	: socket(socket), mps(mps), login_system(ls), debug(false) {}

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
void smtpservice::run_protocol(net::csocket& con_sock) {
	stream socket_stream = con_sock.get_stream();
	std::string line;

	do {
		// Read line by line using new streaming and socket API.
		// Rethrow any uncaught exceptions into the main procedure.
		line = socket_stream.readline();
	
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
			bool login_success = login();

			// Set mutex and update failed/success login counter
			raiilock lck(smtpservice::login_attempts_mutex);
			if (!login_success) {
				smtpservice::login_attempts[usr.get_username()]++;
				try_send_error(socket_stream);
			} else {
				smtpservice::login_attempts[usr.get_username()] = 0;
				try_send_ok(socket_stream);
			}
		}

		// Check if user is authenticated
		if (!usr.is_logged_in()) {
			continue;
		}

		// Start correct mail protocol
		run_smtp_protocols(line);
	} while (line != "quit");

	// Closing the socket
	quit();
	std::cout << "Closing socket ID-" << con_sock.get_handler_id() << std::endl;
	con_sock.close();
}


void smtpservice::run_smtp_protocols(std::string line) {
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

	if (line == "LOGOUT") {
		logout();
	}
}


bool smtpservice::login() {
	stream in = socket.get_stream();

	// Update user and try logging in
	try {
		// TODO: If user is logged in, log out
		if (usr.is_logged_in()) {
			login_system.logout(usr);
		}

		usr.set_username(in.readline());
		usr.set_password(in.readline());

		if (debug) {
			std::cout << "(DM) LOGIN Protocol: " << usr.get_username() << std::endl;
			std::cout << "(DM) LOGIN Protocol: PW: *****" << std::endl;
			std::cout << "(DM) LOGIN Protocol: " << usr.is_fhtw_user() << std::endl;
		}

		// TODO: Run LDAP login procedure here
		login_system.login(usr);
	} catch (std::exception& ex) {
		// An error occurred
		std::cout << ex.what() << std::endl;
		try_send_error(in);
	}
	return usr.is_logged_in();
}


void smtpservice::logout() {
	stream in = socket.get_stream();

	// Try to log the user out
	try {
		if (!login_system.logout(usr)) {
			std::cout << "(DM) LOGOUT Protocol: Not able to logout" << std::endl;
		}
	} catch (std::exception& ex) {
		// An error occurred
		std::cout << ex.what() << std::endl;
		try_send_error(in);
		return;
	}
	try_send_ok(in);
}


void smtpservice::send() {
	stream in = socket.get_stream();

	// Create E-Mail
	email mail;
	try {
		mail.set_sender(usr.get_username());
		mail.set_receiver(in.readline());
		mail.set_subject(in.readline());

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
			message << in.readline();
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
	try_send_ok(in);
}


void smtpservice::list() {
	stream in = socket.get_stream();

	try {
		std::string username = usr.get_username();
		std::vector<email> mails = mps.load_user_mails(username);

		if (debug) {
			std::cout << "(DM) LIST Protocol: " << username << std::endl;
		}

		// List total amount of messages
		std::stringstream ss;
		ss << mails.size() << '\n';
		in.writeline(ss.str());

		// List all messages with numbers
		for (email current : mails) {
			ss.str(std::string());
			ss << current.get_subject() << '\n';
			in.writeline(ss.str());
		}
	} catch(std::exception& ex) {
		std::cout << ex.what() << std::endl;
		try_send_error(in);
	}
}


void smtpservice::read() {
	stream in = socket.get_stream();

	try {
		std::string username = usr.get_username();
		int msg_num = atoi(in.readline().c_str());

		if (debug) {
			std::cout << "(DM) READ Protocol: " << username << std::endl;
			std::cout << "(DM) READ Protocol: " << msg_num << std::endl;
		}

		// Load mail
		email mail = mps.load_mail(username, msg_num);

		// Nothing has thrown an exception, everything is fine so far
		try_send_ok(in);

		// Send E-Mail message
		std::stringstream ss;
		ss.str(std::string());
		ss << mail.get_message();
		ss << std::endl;
		in.writeline(ss.str());
	} catch(std::exception& ex) {
		std::cout << ex.what() << std::endl;
		try_send_error(in);
	}
}


void smtpservice::del() {
	stream in = socket.get_stream();

	try {
		std::string username = usr.get_username();
		int msg_num = atoi(in.readline().c_str());

		if (debug) {
			std::cout << "(DM) DEL Protocol: " << username << std::endl;
			std::cout << "(DM) DEL Protocol: " << msg_num << std::endl;
		}

		// Delete mail
		if (mps.delete_mail(username, msg_num)) {
			try_send_ok(in);
		} else {
			try_send_error(in);
		}
	} catch(std::exception& ex) {
		std::cout << ex.what() << std::endl;
		try_send_error(in);
	}
}


void smtpservice::quit() {
	std::string username = usr.get_username().empty() ? "ANONYMOUS" : usr.get_username();
	std::cout << "User \"" << username << "\" quits the mail server!" << std::endl;
	try {
		socket.get_stream().writeline("Disconnected from the server!\n");
	} catch (std::exception& ex) {
		if (debug) {
			std::cout << "(DM) Socket already closed: " << ex.what() << std::endl;
		}
	}
}


void smtpservice::try_send_ok(stream& in) {
	// Try to send the success code, but catch the exception which may
	// occur if the socket has been closed by the client side.
	try {
		in.writeline("OK\n");
	} catch (std::exception& ex) {
		std::cout << ex.what() << std::endl;
	}
}


void smtpservice::try_send_error(stream& in) {
	// Try to send the error code, but catch the exception which may
	// occur if the socket has been closed by the client side.
	try {
		in.writeline("ERR\n");
	} catch (std::exception& ex) {
		std::cout << ex.what() << std::endl;
	}
}

