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
#include <vector>
#include <stdexcept>


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

	bool file_append_text(std::string file, std::string text, bool linefeed) {
		std::ofstream out;
		out.open(file, std::ofstream::out | std::ofstream::app);
		out << text;
		if (linefeed) {
			out << std::endl;
		}
		out.close();
		return true;
	}

	std::vector<std::string> list_files(std::string dir) {
		DIR* c_dir;
		struct dirent* entry;

		// Open directory
		if ((c_dir = opendir(dir.c_str())) == NULL) {
			throw std::runtime_error("Could not open directory: " + dir);
		}

		// Read all files from directory
		std::vector<std::string> result;
		while((entry = readdir(c_dir)) != NULL) {
			std::string d_name(entry->d_name);
			if (d_name == "." || d_name == "..") {
				continue;
			}
			result.push_back(d_name);
		}

		// Close directory
		closedir(c_dir);
		return result;
	}

	bool move_dir(std::string old_dir, std::string new_dir) {
		return rename(old_dir.c_str(), new_dir.c_str()) == 0;
	}
}
