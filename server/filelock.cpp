/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 16.10.2017
 *
 * (c) All rights reserved
 */


#include "filelock.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>

#include <string>
#include <stdexcept>



filelock::filelock(std::string file_name)
	: fd(-1), locked(false), fn(file_name) {}


filelock::~filelock() {
	// Automatically unlock on resource release
	if (locked) {
		unlock();
	}
}



bool filelock::try_lock(bool block) {
	// Try to open the file
	fd = open(fn.c_str(), O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);

	// File not found
	if (fd < 0) {
		throw std::runtime_error("Can't open file for lock!");
	}

	// Create lock mask
	int lock_mask = LOCK_EX;
	if (!block) {
		lock_mask |= LOCK_NB;
	}

	// Try to obtain the file lock
	locked = flock(fd, lock_mask) == 0;
	if (!locked) {
		close(fd);
		fd = -1;
	}
	return locked;
}


void filelock::unlock() {
	if (!locked || fd < 0) {
		throw std::runtime_error("Lock not obtained");
	}

	// Close and remove lock
	remove(fn.c_str());
	close(fd);
	fd = -1;
	locked = false;
}
