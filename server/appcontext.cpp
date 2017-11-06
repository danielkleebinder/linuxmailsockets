/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 30.10.2017
 *
 * (c) All rights reserved
 */


#include "appcontext.h"

#include <iostream>
#include <stdexcept>
#include <map>
#include <memory>
#include <string>
#include <sstream>

#include "mail/mailpoolservice.h"
#include "net/serversocket.h"



// App Context private constructor and destructor
appcontext::appcontext() {}
appcontext::~appcontext() {}


// Variables
bool appcontext::initialized = false;
bool appcontext::debug_mode = false;
net::serversocket* appcontext::ss;
std::map<std::string, std::shared_ptr<appcontext::attempt_t>>* appcontext::blacklist;



// Initializes the app context
void appcontext::initialize(int port, mailpoolservice& mps) {
	if (initialized) {
		throw std::runtime_error("App Context was already initialized");
	}

	// Load serversocket
	ss = new net::serversocket(port);

	// Load blacklist
	blacklist = new std::map<std::string, std::shared_ptr<appcontext::attempt_t>>();
	auto bl = mps.load_blacklist();
	for (auto const& entry : bl) {
		auto sp = std::make_shared<appcontext::attempt_t>();
		appcontext::attempt_t* at = sp.get();
		at->num_attempts = 3;
		at->last_sec = entry.second;
		(*blacklist)[entry.first] = sp;
	}

	// Everything is initialized
	initialized = true;
}

// Returns if the context was already initialized
bool appcontext::is_initialized() {
	return initialized;
}

// Returns the server socket
net::serversocket* appcontext::get_serversocket() {
	if (!initialized) {
		throw std::runtime_error("App Context was not initialized yet");
	}
	return ss;
}

// Returns the blacklist
std::map<std::string, std::shared_ptr<struct appcontext::attempt_t>>* appcontext::get_blacklist() {
	if (!initialized) {
		throw std::runtime_error("App Context was not initialized yet");
	}
	return blacklist;
}

// Enables or disables the debug mode
void appcontext::set_debug_mode(bool debug_mode) {
	appcontext::debug_mode = debug_mode;
}

// Returns the debug mode
bool appcontext::is_debug_mode() {
	return debug_mode;
}

// Outputs a debug message on layer 0
void appcontext::debug_log(std::string msg) {
	debug_log(msg, 0);
}

// Outputs a debug message if the debug mode is enabled
void appcontext::debug_log(std::string msg, int layer) {
	if (!debug_mode) {
		return;
	}

	// Generate debug output
	std::stringstream ss;
	ss << "(DM)";
	for (int i = 0; i < layer; i++) {
		ss << "  ";
	}
	if (layer > 0) {
		ss << "->";
	}
	ss << " " << msg;

	// Print message
	std::cout << ss.str() << std::endl;
}

// Serializes the blacklist to the given mail pool service
void appcontext::serialize_blacklist(mailpoolservice& mps) {
	// Save blacklist
	std::map<std::string, time_t> save;
	for (auto const& entry : *blacklist) {
		auto sp = entry.second;
		attempt_t* at = sp.get();
		if (at->num_attempts >= 3) {
			save[entry.first] = at->last_sec;
		}
	}

	// Use mailpoolservice
	mps.save_blacklist(save);
}

// Disposes the app context
void appcontext::dispose() {
	delete ss;
	delete blacklist;

	ss = NULL;
	blacklist = NULL;

	initialized = false;
}
