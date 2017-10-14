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
#include <cstring>


namespace fs {
	bool exists(std::string fd) {
		struct stat info;
		return stat(fd.c_str(), &info) == 0;
	}

	bool make_dir(std::string dir) {
		return mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == 0;
	}

	bool make_dir_rec(std::string dir) {
		const char* c_dir = dir.c_str();
		char tmp[256];
		char* p = NULL;
		size_t len;

		snprintf(tmp, sizeof(tmp), "%s", c_dir);
		len = strlen(tmp);
		if (tmp[len - 1] == '/') {
			tmp[len - 1] = 0;
		}

		for (p = tmp + 1; *p; p++) {
			if (*p == '/') {
				*p = 0;
				if (!make_dir(std::string(tmp))) {
					return false;
				}
				*p = '/';
			}
		}
		return make_dir(std::string(tmp));
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
