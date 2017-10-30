#ifndef APPCONTEXT_H
#define APPCONTEXT_H

#include "net/serversocket.h"

/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 30.10.2017
 *
 * (c) All rights reserved
 */
class appcontext final {
public:
	virtual ~appcontext();

	static void initialize(int port);
	static bool is_initialized();

	static net::serversocket* get_serversocket();

	static void dispose();

private:
	appcontext();

	static bool initialized;
	static net::serversocket* ss;
};

#endif // APPCONTEXT_H
