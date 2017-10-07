#ifndef NETUTILS_H
#define NETUTILS_H

#include <unistd.h>
#include <string>


/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 30.09.2017
 *
 * (c) All rights reserved
 */
namespace netutils {
	std::string readline(int file_descriptor);
}

#endif // NETUTILS_H
