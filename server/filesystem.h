#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <unistd.h>
#include <string>


/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 30.09.2017
 *
 * (c) All rights reserved
 */
namespace fs {
	bool exists(std::string fd);
	bool make_dir(std::string dir);
	bool make_file(std::string file);
	bool file_append_text(std::string file, std::string text);
}

#endif // FILESYSTEM_H
