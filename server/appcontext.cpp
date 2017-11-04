/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 30.10.2017
 *
 * (c) All rights reserved
 */


#include "appcontext.h"

#include <stdexcept>
#include <map>
#include <memory>
#include <string>

#include "mail/mailpoolservice.h"
#include "net/serversocket.h"



// App Context private constructor and destructor
appcontext::appcontext() {}
appcontext::~appcontext() {}


// Variables
bool appcontext::initialized = false;
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
