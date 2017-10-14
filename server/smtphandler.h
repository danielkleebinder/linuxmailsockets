#ifndef SMTPHANDLER_H
#define SMTPHANDLER_H


#include "net/socket.h"


/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 08.10.2017
 *
 * (c) All rights reserved
 */
class smtphandler {
public:
	smtphandler(net::socket& socket);
	~smtphandler();

	void send();
	void list();
	void read();
	void del();
	void quit();

private:
	net::socket& socket;
};


#endif // SMTPHANDLER_H
