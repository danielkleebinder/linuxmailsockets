#ifndef SMTPSERVICE_H
#define SMTPSERVICE_H


#include "mailpoolservice.h"
#include "../net/socket.h"


/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 08.10.2017
 *
 * (c) All rights reserved
 */
class smtpservice {
public:
	smtpservice(net::ssocket& socket, mailpoolservice& mps);
	~smtpservice();

	// Starts the SMTP protocol in an own thread
	void start_forked_service();
	// Starts the SMTP protocol in the main thread
	void start_service();

protected:
	// Standard SMTP functions are virtual for newer
	// protocols to override and reimplement them.
	virtual void send();
	virtual void list();
	virtual void read();
	virtual void del();
	virtual void quit();

	// Run the whole SMTP protocol
	void run_protocol(net::ssocket& con_sock);

private:
	net::ssocket& socket;
	mailpoolservice& mps;
};


#endif // SMTPSERVICE_H
