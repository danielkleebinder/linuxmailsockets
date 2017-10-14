/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 14.10.2017
 *
 * (c) All rights reserved
 */


#include "raiilock.h"
#include <mutex>


raiilock::raiilock(std::mutex& m) : _mutex(m) {
	m.lock();
}

raiilock::~raiilock() {
	_mutex.unlock();
}
