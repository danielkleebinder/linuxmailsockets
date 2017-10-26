#ifndef SMTPSERVICE_H
#define SMTPSERVICE_H


#include "mailpoolservice.h"
#include "../net/socket.h"
#include "../net/stream.h"
#include "../user.h"
#include "../login/loginsystem.h"

#include <mutex>
#include <map>


/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 08.10.2017
 *
 * (c) All rights reserved
 */
class smtpservice {
public:
	smtpservice(net::ssocket& socket, mailpoolservice& mps, loginsystem& ls);
	~smtpservice();

	// Starts the SMTP protocol in an own thread
	void start_forked_service();
	// Starts the SMTP protocol in the main thread
	void start_service();

	// Allow debug mode
	void set_debug_mode(bool debug);
	bool get_debug_mode();

protected:
	// Standard SMTP functions are virtual for newer
	// protocols to override and reimplement them.
	virtual bool login();
	virtual void logout();
	virtual void send();
	virtual void list();
	virtual void read();
	virtual void del();
	virtual void quit();

	// Run the whole SMTP protocol
	void run_protocol(net::ssocket& con_sock);
	void run_smtp_protocols(std::string line);

private:
	// Map and mutex for login attempts counter
	static std::map<std::string, int> login_attempts;
	static std::mutex login_attempts_mutex;

	// Class variables
	net::ssocket& socket;
	mailpoolservice& mps;
	loginsystem& login_system;

	user usr;
	bool debug;

	// Private send ok and send error methods
	void try_send_ok(stream& in);
	void try_send_error(stream& in);
};


#endif // SMTPSERVICE_H
