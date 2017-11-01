/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 30.10.2017
 *
 * (c) All rights reserved
 */


#include "appcontext.h"

#include <stdexcept>
#include "net/serversocket.h"



// App Context private constructor and destructor
appcontext::appcontext() {}
appcontext::~appcontext() {}


// Variables
bool appcontext::initialized = false;
net::serversocket* appcontext::ss;

// Initializes the app context
void appcontext::initialize(int port) {
	if (initialized) {
		throw std::runtime_error("App Context was already initialized");
	}
	ss = new net::serversocket(port);
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

// Disposes the app context
void appcontext::dispose() {
	delete ss;
	ss = NULL;
	initialized = false;
}
