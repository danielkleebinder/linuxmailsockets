/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 30.09.2017
 *
 * (c) All rights reserved
 */


#include "raiilock.h"

#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <stdexcept>
#include <mutex>


// Contains utility methods for file access.
namespace fs {

	// Globally used file lock mutex
	static std::mutex _mutex;


	/**
	 * Checks if the given file exists.
	 *
	 * @param fd File descriptor (handler).
	 * @return True if file exsist, otherwise false.
	 */
	bool exists(std::string fd) {
		struct stat info;
		return stat(fd.c_str(), &info) == 0;
	}


	/**
	 * Creates a new directory.
	 *
	 * @param dir Directory.
	 * @return True if directory creation was successful.
	 */
	bool make_dir(std::string dir) {
		return mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == 0;
	}


	/**
	 * Creates directories recursivly.
	 *
	 * @param dir Directory.
	 * @return True if directory creation was successful.
	 */
	bool make_dir_rec(std::string dir) {
		char tmp[256];
		char* p = NULL;
		size_t len;

		// Retreive first part of directory hierarchy
		snprintf(tmp, sizeof(tmp), "%s", dir.c_str());
		len = strlen(tmp);
		if (tmp[len - 1] == '/') {
			tmp[len - 1] = 0;
		}

		// Create directories and lock this process
		{
			raiilock sl(_mutex);
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
	}


	/**
	 * Creates the given file.
	 *
	 * @param file File.
	 * @return True if file was successfully created.
	 */
	bool make_file(std::string file) {
		std::ofstream out(file);
		return true;
	}


	/**
	 * Appends the given text to the file.
	 *
	 * @param file File to append text on.
	 * @param text Text.
	 * @param linefeed If a line feed should be appended at the end of the text-
	 * @return True if text was successfully appended.
	 */
	bool file_append_text(std::string file, std::string text, bool linefeed) {
		{
			raiilock sl(_mutex);
			std::ofstream out;
			out.open(file, std::ofstream::out | std::ofstream::app);
			out << text;
			if (linefeed) {
				out << std::endl;
			}
			out.close();
		}
		return true;
	}


	/**
	 * Writes the given byte array into the given file.
	 *
	 * @param file File to write bytes to.
	 * @param bytes Bytes to be written.
	 * @return True if the byte array was successfully written.
	 */
	bool file_write_bytes(std::string file, uint8_t* bytes) {
		{
			raiilock s1(_mutex);
			std::ofstream out;
			out.open(file, std::ofstream::out);
			out.write((char*) bytes, sizeof(bytes));
			out.close();
		}
		return true;
	}


	/**
	 * Lists all files and directories in the given directory.
	 *
	 * @param dir Directory.
	 * @return List of all files (directories and files).
	 */
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


	/**
	 * Moves a given directory to another one.
	 *
	 * @param old_dir Old directory which should be moved.
	 * @param new_dir New directory to move to.
	 * @return True if directory was successfully moved.
	 */
	bool move_dir(std::string old_dir, std::string new_dir) {
		raiilock sl(_mutex);
		return rename(old_dir.c_str(), new_dir.c_str()) == 0;
	}
}
