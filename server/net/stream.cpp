/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 10.10.2017
 *
 * (c) All rights reserved
 */


#include "stream.h"

#include <string>
#include <stdexcept>
#include <unistd.h>


stream::stream(int handler)
	: _handler(handler) {}

stream::~stream() {}


std::string stream::get_host() {
	return _host;
}

int stream::get_port() {
	return _port;
}

void stream::close() {
	close(_handler);
}


char stream::read() {
	char ch;
	int n = read(stream_handler, &ch, 1);
	if (n == -1) {
		throw exception("Error while reading a character from the stream");
	}
	return ch;
}

std::string stream::readline() {
	std::string result;
	char ch;
	int n = 0;

	// Read characters from stream until a line feed
	// was found.
	while ((n = read(_handler, &ch, 1)) != 0) {
		if (n == -1) {
			return NULL;
		}

		// Check for end of line
		if (ch == '\n' || ch == '\r' || ch == '\0') {
			break;
		}

		// Concat character
		result += ch;
	}

	// Return the string result
	return result;	
}

void stream::write(char ch) {
	int n = write(_handler, ch, 1);
	if (n < 1) {
		throw exception("Error while writing a character to the stream");
	}
}

void stream::write(std::string str) {
	int size = str.length();
	int n = write(_handler, str.c_str(), size);
	if (n < size) {
		throw exception("Error while writing a text to the stream");
	}
}

