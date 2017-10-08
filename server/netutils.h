#ifndef NETUTILS_H
#define NETUTILS_H

#include <unistd.h>
#include <string>


/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 07.10.2017
 *
 * (c) All rights reserved
 */
namespace netutils {
	std::string readline(int file_descriptor);
	bool writetext(int file_descriptor, std::string text);
}

#endif // NETUTILS_H
