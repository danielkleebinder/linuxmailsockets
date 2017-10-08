#ifndef SMTPHANDLER_H
#define SMTPHANDLER_H


/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 08.10.2017
 *
 * (c) All rights reserved
 */
class smtphandler {
public:
	smtphandler(int socket);
	~smtphandler();

	void send();
	void list();
	void read();
	void del();
	void quit();

private:
	int socket;
};


#endif // SMTPHANDLER_H
