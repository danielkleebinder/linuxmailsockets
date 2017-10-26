#ifndef LDAPLOGIN_H
#define LDAPLOGIN_H


#include "loginsystem.h"
#include "../user.h"

/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 26.10.2017
 *
 * (c) All rights reserved
 */
class ldaplogin : public loginsystem {
public:
	ldaplogin();
	virtual ~ldaplogin();

	// Interface methods for a basic login system
	virtual bool login(user& usr);
	virtual bool logout(user& usr);
};

#endif // LDAPLOGIN_H
