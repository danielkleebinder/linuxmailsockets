#ifndef LOGINSYSTEM_H
#define LOGINSYSTEM_H

#include "../user.h"

/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 26.10.2017
 *
 * (c) All rights reserved
 */
class loginsystem {
public:
	// Interface methods for a basic login system
	virtual bool login(user& usr) = 0;
	virtual bool logout(user& usr) = 0;
};

#endif // LOGINSYSTEM_H
