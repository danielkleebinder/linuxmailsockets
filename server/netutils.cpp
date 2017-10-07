/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 30.09.2017
 *
 * (c) All rights reserved
 */


#include <unistd.h>
#include <string>

#include <iostream>


namespace netutils {

	/*
	* Reads exactly one line from the given file descriptor.
	*
	* @param file_descriptor C file descriptor (socket for example).
	* @return Read line.
	*/
	std::string readline(int file_descriptor) {
		std::string result;
		char ch;
		int n = 0;

		// Read characters from stream until a line feed
		// was found.
		while ((n = read(file_descriptor, &ch, 1)) != 0) {
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
}
