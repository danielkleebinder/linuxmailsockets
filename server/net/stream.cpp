/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 10.10.2017
 *
 * (c) All rights reserved
 */


#include "stream.h"

#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <stdexcept>
#include <unistd.h>



stream::stream(int handler)
	: _handler(handler) {}

stream::~stream() {}


void stream::close_stream() {
	close(_handler);
}


char stream::sread() {
	char ch;
	int n = read(_handler, &ch, 1);
	if (n == -1) {
		throw std::runtime_error("Error while reading a character from the stream");
	}
	return ch;
}

std::string stream::sreadline() {
	std::string result;
	char ch;
	int n = 0;

	// Read characters from stream until a line feed
	// was found.
	while ((n = read(_handler, &ch, 1)) != 0) {
		if (n == -1) {
			break;
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

void stream::swrite(char ch) {
	int n = write(_handler, &ch, 1);
	if (n < 1) {
		throw std::runtime_error("Error while writing a character to the stream");
	}
}

void stream::swrite(std::string str) {
	int size = str.length();
	int n = write(_handler, str.c_str(), size);
	if (n < size) {
		throw std::runtime_error("Error while writing a text to the stream");
	}
}

