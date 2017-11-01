/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 08.10.2017
 *
 * (c) All rights reserved
 */


#include "../user.h"
#include "../raiilock.h"
#include "../appcontext.h"
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
#include <memory>


// Initialize static variables
std::mutex smtpservice::login_attempts_mutex;


// SMTP Service constructor
smtpservice::smtpservice(net::csocket* socket, mailpoolservice& mps, loginsystem& ls)
	: socket(socket), mps(mps), login_system(ls), debug(false), timeout(300) {}

smtpservice::~smtpservice() {}


void smtpservice::start_forked_service() {
	std::thread con_thread(&smtpservice::run_protocol, this, socket);
	con_thread.detach();
}


void smtpservice::start_service() {
	run_protocol(socket);
}


void smtpservice::set_debug_mode(bool debug) {
	this->debug = debug;
}


bool smtpservice::get_debug_mode() {
	return debug;
}


void smtpservice::set_timeout(time_t sec) {
	timeout = sec;
}


time_t smtpservice::get_timeout() {
	return timeout;
}


bool smtpservice::is_address_blocked(std::string addr) {
	raiilock lck(smtpservice::login_attempts_mutex);
	if (appcontext::get_blacklist()->find(addr) != appcontext::get_blacklist()->end()) {
		appcontext::attempt_t* at = (*appcontext::get_blacklist())[addr].get();
		if (at->num_attempts >= 3) {
			return (time(NULL) - at->last_sec) <= timeout;
		}
	}
	return false;
}


time_t smtpservice::get_address_timeout(std::string addr) {
	raiilock lck(smtpservice::login_attempts_mutex);
	if (appcontext::get_blacklist()->find(addr) != appcontext::get_blacklist()->end()) {
		appcontext::attempt_t* at = (*appcontext::get_blacklist())[addr].get();
		if (at->num_attempts >= 3) {
			return time(NULL) - at->last_sec;
		}
	}
	return 0;
}



/**
 * Handles the connection from a client.
 *
 * @param con_sock Connected client socket.
 */
void smtpservice::run_protocol(net::csocket* con_sock) {
	stream s = con_sock->get_stream();
	std::string line;

	// Get socket information
	std::string ip = con_sock->get_address();
	int port = con_sock->get_port();

	do {
		// Read line by line using new streaming and socket API.
		// Rethrow any uncaught exceptions into the main procedure.
		line = s.readline();
	
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
			bool contains_ip = false;
			bool blocked = is_address_blocked(ip);
			time_t diff = get_address_timeout(ip);

			// Obtain lock and check login attempts
			{
				raiilock lck(smtpservice::login_attempts_mutex);
				contains_ip = appcontext::get_blacklist()->find(ip) != appcontext::get_blacklist()->end();
			}

			// Skip login if the user is blocked
			if (blocked) {
				if (debug) {
					std::cout << "  -> (DM) IP address (" << ip << ":" << port << ") is temporarily blocked for " << (timeout - diff) << " seconds" << std::endl;
				}
				try_send_error(s);
				quit();
				con_sock->close();
				delete con_sock;
				serialize_blacklist();
				return;
			}

			// Try user login
			bool login_success = login();

			// Check if ip entry already exists
			if (!contains_ip) {
				auto sp = std::make_shared<appcontext::attempt_t>();
				appcontext::attempt_t* at = sp.get();
				at->num_attempts = 0;

				// Obtain lock and set attempt
				raiilock lck(smtpservice::login_attempts_mutex);
				(*appcontext::get_blacklist())[ip] = sp;
			}

			// Check if login was successful
			if (!login_success) {
				raiilock lck(smtpservice::login_attempts_mutex);
				
				// Get attempt in locked environment
				appcontext::attempt_t* at = (*appcontext::get_blacklist())[ip].get();
				at->num_attempts++;
				at->last_sec = time(NULL);
				try_send_error(s);
			} else {
				raiilock lck(smtpservice::login_attempts_mutex);
				(*appcontext::get_blacklist())[ip].get()->num_attempts = 0;
				try_send_ok(s);
			}

			// Print login attempt debug output
			if (debug) {
				raiilock lck(smtpservice::login_attempts_mutex);
				appcontext::attempt_t* at = (*appcontext::get_blacklist())[ip].get();
				std::cout << "  -> (DM) Login Attempt by " << ip << ":" << port << " - Nr.: " << at->num_attempts << ", Last: " << at->last_sec << std::endl;
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
	std::cout << "Closing socket ID-" << con_sock->get_handler_id() << " from " << ip << ":" << port << std::endl;
	con_sock->close();
	delete con_sock;
	serialize_blacklist();
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
	stream in = socket->get_stream();

	// Update user and try logging in
	try {
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

		login_system.login(usr);
	} catch (std::exception& ex) {
		// An error occurred
		std::cout << ex.what() << std::endl;
		try_send_error(in);
	}
	return usr.is_logged_in();
}


void smtpservice::logout() {
	stream in = socket->get_stream();

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
	stream in = socket->get_stream();

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

		// Read attachments protocol
		uint16_t num_attachments = in.readuint16();
		for (uint16_t i = 0; i < num_attachments; i++) {
			std::string name = in.readline();

			uint64_t num_bytes = in.readuint64();

			std::shared_ptr<uint8_t> sp(new uint8_t[num_bytes], std::default_delete<uint8_t[]>());
			uint8_t* bytes = sp.get();
			in.readbytes(bytes, num_bytes);

			attachment att;
			att.set_name(name);
			att.set_data_ptr(sp);
			mail.get_attachments().push_back(att);
		}

		// Save E-Mail
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
	stream in = socket->get_stream();

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
	stream in = socket->get_stream();

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
	stream in = socket->get_stream();

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
		socket->get_stream().writeline("Disconnected from the server!\n");
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


void smtpservice::serialize_blacklist() {
	raiilock lck(smtpservice::login_attempts_mutex);
	appcontext::serialize_blacklist(mps);
}
