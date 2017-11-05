/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 10.10.2017
 *
 * (c) All rights reserved
 */


#include "stream.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <stdexcept>
#include <unistd.h>
#include <sstream>
#include <cstdint>



stream::stream(int handler)
	: _handler(handler) {}

stream::~stream() {}


void stream::close_stream() {
	close(_handler);
}


char stream::readchar() {
	return (char) readbyte();
}


std::string stream::readline() {
	std::stringstream result;
	char ch;
	int n = 0;

	// Read characters from stream until a line feed
	// was found.
	while ((n = read(_handler, &ch, 1)) != 0) {
		if (n == -1) {
			break;
		}

		// Skip carriage return for DOS systems and programs like telnet
		if (ch == '\r') {
			continue;
		}

		// Check for end of line
		if (ch == '\n'|| ch == '\0') {
			break;
		}

		// Concat character
		result << ch;
	}

	// Return the string result
	return result.str();	
}


void stream::writechar(char ch) {
	writebyte((uint8_t) ch);
}


void stream::writeline(std::string str) {
	int size = str.length();
	int n = write(_handler, str.c_str(), size);
	if (n < size) {
		throw std::runtime_error("Error while writing a text to the stream");
	}
}


uint8_t stream::readbyte() {
	uint8_t result;
	readbytes(&result, 1);
	return result;
}


void stream::readbytes(uint8_t* bytes, int n) {
	ssize_t c = read(_handler, bytes, n);
	if (c < 1) {
		throw std::runtime_error("Error while reading bytes from the stream");
	}
}


void stream::writebyte(uint8_t b) {
	writebytes(&b, 1);
}


void stream::writebytes(uint8_t* bytes, int n) {
	writebytes(bytes, 0, n);
}


void stream::writebytes(uint8_t* bytes, int offset, int size) {
	// uint8_t is exactly one byte, no need for sizeof operator
	ssize_t n = write(_handler, bytes + offset, size);
	if (n < size) {
		throw std::runtime_error("Error while writing bytes to the stream");
	}
}


uint16_t stream::readuint16() {
	uint16_t result;
	readbytes((uint8_t*) &result, 2);
	return result;
}


uint32_t stream::readuint32() {
	uint32_t result;
	readbytes((uint8_t*) &result, 4);
	return result;
}


uint64_t stream::readuint64() {
	uint64_t result;
	readbytes((uint8_t*) &result, 8);
	return result;

}


void stream::writeuint16(uint16_t v) {
	writebytes((uint8_t*) &v, 2);
}


void stream::writeuint32(uint32_t v) {
	writebytes((uint8_t*) &v, 4);
}


void stream::writeuint64(uint64_t v) {
	writebytes((uint8_t*) &v, 8);
}
