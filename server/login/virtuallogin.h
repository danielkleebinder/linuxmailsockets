#ifndef VIRTUALLOGIN_H
#define VIRTUALLOGIN_H


#include "loginsystem.h"
#include "../user.h"

/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 26.10.2017
 *
 * (c) All rights reserved
 */

// A virtual login class for test fake logins
class virtuallogin : public loginsystem {
public:
	virtuallogin();
	virtual ~virtuallogin();

	// Interface methods for a basic login system
	virtual bool login(user& usr);
	virtual bool logout(user& usr);
};

#endif // VIRTUALLOGIN_H
