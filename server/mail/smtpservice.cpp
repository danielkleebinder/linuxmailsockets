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
#include <string.h>
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
	: socket(socket), mps(mps), login_system(ls), timeout(300) {}

smtpservice::~smtpservice() {}


void smtpservice::start_forked_service() {
	std::thread con_thread(&smtpservice::run_protocol, this, socket);
	con_thread.detach();
}


void smtpservice::start_service() {
	run_protocol(socket);
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

	
	bool blocked = is_address_blocked(ip);
	if (blocked) {
		time_t diff = get_address_timeout(ip);
		appcontext::debug_log("IP address (" + ip + ":" + std::to_string(port) + ") is temporarily blocked for " + std::to_string(timeout - diff) + " seconds", 1);
		try_send_error(s);
		quit();
		release_resources();
		return;
	}

	// Send ok if the client was successfully conncted
	try_send_ok(s);
	do {
		// Read line by line using new streaming and socket API.
		// Rethrow any uncaught exceptions into the main procedure.
		line = s.readline();

		// Output debug
		appcontext::debug_log("Read line: " + line);

		// Check if line is null or empty
		if (line.empty()) {
			continue;
		}

		// Login user before any other protocol can be executed
		if (line == "LOGIN") {
			bool contains_ip = false;
			blocked = is_address_blocked(ip);
			time_t diff = get_address_timeout(ip);

			// Obtain lock and check login attempts
			{
				raiilock lck(smtpservice::login_attempts_mutex);
				contains_ip = appcontext::get_blacklist()->find(ip) != appcontext::get_blacklist()->end();
			}

			// Skip login if the user is blocked
			if (blocked) {
				appcontext::debug_log("IP address (" + ip + ":" + std::to_string(port) + ") is temporarily blocked for " + std::to_string(timeout - diff) + " seconds", 1);
				try_send_error(s);
				quit();
				con_sock->close();
				delete con_sock;
				serialize_blacklist();
				return;
			}

			// Try user login
			bool login_success = login();
			appcontext::debug_log("Login successful: " + std::to_string(login_success), 1);

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
			if (appcontext::is_debug_mode()) {
				raiilock lck(smtpservice::login_attempts_mutex);
				appcontext::attempt_t* at = (*appcontext::get_blacklist())[ip].get();
				appcontext::debug_log("Login Attempt by " + ip + ":" + std::to_string(port) + " - Nr.: " + std::to_string(at->num_attempts) + ", Last: " + std::to_string(at->last_sec));
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

	if (line == "ATT") {
		att();
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

		appcontext::debug_log("Start [LOGIN] Protocol");
		appcontext::debug_log("Username: " + usr.get_username(), 1);
		appcontext::debug_log("Password: ********", 1);
		appcontext::debug_log("FHTW Usr: " + std::to_string(usr.is_fhtw_user()), 1);

		login_system.login(usr);

		appcontext::debug_log("End [LOGIN] Protocol");
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
		appcontext::debug_log("Start [LOGOUT] Protocol");
		if (!login_system.logout(usr)) {
			appcontext::debug_log("Not able to logout", 1);
		}
		appcontext::debug_log("End [LOGOUT] Protocol");
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

		appcontext::debug_log("Start [SEND] Protocol");
		appcontext::debug_log("Sender: " + mail.get_sender(), 1);
		appcontext::debug_log("Receiver: " + mail.get_receiver(), 1);
		appcontext::debug_log("Subject: " + mail.get_subject(), 1);

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

		appcontext::debug_log("Finished reading mail message", 1);

		// Read attachments protocol
		uint8_t num_attachments = in.readbyte();
		appcontext::debug_log("Number of attachments to read: " + std::to_string(unsigned(num_attachments)), 1);
		for (uint8_t i = 0; i < num_attachments; i++) {
			std::string name = in.readline();

			uint64_t num_bytes = in.readuint64();

			// Print debug log
			appcontext::debug_log("Attachment \"" + name + "\" with size (in bytes): " + std::to_string(unsigned(num_bytes)), 2);

			// Read data
			std::shared_ptr<uint8_t> sp(new uint8_t[num_bytes], std::default_delete<uint8_t[]>());
			uint8_t* bytes = sp.get();
			in.readbytesfull(bytes, num_bytes);

			// Create attachment
			attachment att;
			att.set_name(name);
			att.set_data_ptr(sp);
			att.set_size(num_bytes);
			mail.get_attachments().push_back(att);
		}

		// Save E-Mail
		mps.save_mail(mail);

		appcontext::debug_log("End [SEND] Protocol");
	} catch (std::exception& ex) {
		// An error occurred
		std::cout << ex.what() << std::endl;
		try_send_error(in);
		return;
	}

	// Everything is fine
	try_send_ok(in);
}


void smtpservice::att() {
	stream in = socket->get_stream();

	try {
		std::string username = usr.get_username();
		int msg_num = atoi(in.readline().c_str());

		appcontext::debug_log("Start [ATT] Protocol");
		appcontext::debug_log("Username: " + username, 1);
		appcontext::debug_log("Message ID: " + std::to_string(msg_num), 1);

		// Load mail
		email mail = mps.load_mail(username, msg_num);

		// Nothing has thrown an exception, everything is fine so far
		try_send_ok(in);

		// Send attachments
		in.writebyte((uint8_t) mail.get_attachments().size());
		appcontext::debug_log("Number of attachments: " + std::to_string(unsigned(mail.get_attachments().size())), 1);
		for (attachment att : mail.get_attachments()) {
			appcontext::debug_log("Attachment \"" + att.get_name() + "\" with size (in bytes): " + std::to_string(unsigned(att.get_size())), 2);

			in.writeline(att.get_name() + '\n');
			in.writeuint64(att.get_size());
			in.writebytes(att.get_data(), (int) att.get_size());
		}
		appcontext::debug_log("End [ATT] Protocol");
	} catch(std::exception& ex) {
		std::cout << ex.what() << std::endl;
		try_send_error(in);
	}
}


void smtpservice::list() {
	stream in = socket->get_stream();

	try {
		std::string username = usr.get_username();
		std::vector<email> mails = mps.load_user_mails(username);

		appcontext::debug_log("Start [LIST] Protocol");
		appcontext::debug_log("Username: " + username, 1);
		appcontext::debug_log("Number of mails available: " + std::to_string(mails.size()), 1);

		// List total amount of messages
		std::stringstream ss;
		ss << mails.size() << '\n';
		in.writeline(ss.str());

		// List all messages with numbers
		for (email current : mails) {
			appcontext::debug_log(current.get_subject(), 2);
			ss.str(std::string());
			ss << current.get_subject() << '\n';
			in.writeline(ss.str());
		}

		appcontext::debug_log("End [LIST] Protocol");
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

		appcontext::debug_log("Start [READ] Protocol");
		appcontext::debug_log("Username: " + username, 1);
		appcontext::debug_log("E-Mail ID: " + std::to_string(msg_num), 1);

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

		appcontext::debug_log("End [READ] Protocol");
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

		appcontext::debug_log("Start [DEL] Protocol");
		appcontext::debug_log("Username: " + username, 1);
		appcontext::debug_log("E-Mail ID: " + std::to_string(msg_num), 1);

		// Delete mail
		if (mps.delete_mail(username, msg_num)) {
			try_send_ok(in);
		} else {
			try_send_error(in);
		}

		appcontext::debug_log("End [DEL] Protocol");
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
		std::cout << "Socket already closed: " << ex.what() << std::endl;
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
	appcontext::debug_log("Serializing IP Blacklist");
	raiilock lck(smtpservice::login_attempts_mutex);
	appcontext::serialize_blacklist(mps);
}


void smtpservice::release_resources() {
	socket->close();
	delete socket;
	serialize_blacklist();
}
