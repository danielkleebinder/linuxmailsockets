#ifndef RAIILOCK_H
#define RAIILOCK_H

#include <mutex>

/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 14.10.2017
 *
 * (c) All rights reserved
 */
class raiilock {
public:
	raiilock(std::mutex& m);
	~raiilock();

private:
	std::mutex& _mutex;
};

#endif // RAIILOCK_H
