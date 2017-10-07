/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 30.09.2017
 *
 * (c) All rights reserved
 */


#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#include <iostream>
#include <fstream>
#include <string>


namespace fs {
	bool exists(std::string fd) {
		struct stat info;
		return stat(fd.c_str(), &info) == 0;
	}

	bool make_dir(std::string dir) {
		return mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == 0;
	}

	bool make_file(std::string file) {
		std::ofstream out(file);
		return true;
	}

	bool file_append_text(std::string file, std::string text) {
		std::ofstream out;
		out.open(file, std::ofstream::out | std::ofstream::app);
		out << text;
		out.close();
		return true;
	}
}
