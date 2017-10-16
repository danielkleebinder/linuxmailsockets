#ifndef FILELOCK_H
#define FILELOCK_H

#include <string>

/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 16.10.2017
 *
 * (c) All rights reserved
 */
class filelock {
public:
	filelock(std::string file_name);
	~filelock();

	bool try_lock(bool block);
	void unlock();

private:
	int fd;
	bool locked;
	std::string fn;
};

#endif // FILELOCK_H
