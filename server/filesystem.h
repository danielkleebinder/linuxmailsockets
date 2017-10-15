#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <unistd.h>
#include <string>
#include <vector>


/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 30.09.2017
 *
 * (c) All rights reserved
 */
namespace fs {
	bool exists(std::string fd);
	bool make_dir(std::string dir);
	bool make_dir_rec(std::string dir);
	bool make_file(std::string file);
	bool file_append_text(std::string file, std::string text, bool linefeed);

	std::vector<std::string> list_files(std::string dir);
	bool move_dir(std::string old_dir, std::string new_dir);
}

#endif // FILESYSTEM_H
