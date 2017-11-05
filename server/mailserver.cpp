/*
 * AUTHOR: KLEEBINDER Daniel, SCHACHNER Thomas
 * CREATED ON: 30.09.2017
 *
 * DESCRIPTION: A low level SMTP server implementation
 *			feeding SMTP clients.
 */



// Include C++/11 Headers
#include <string>
#include <iostream>
#include <memory>

// Include C system libaries
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// Include custom classes
#include "appcontext.h"

#include "login/loginsystem.h"
#include "login/virtuallogin.h"
#include "login/ldaplogin.h"

#include "net/stream.h"
#include "net/socket.h"
#include "net/serversocket.h"

#include "mail/email.h"
#include "mail/attachment.h"
#include "mail/smtpservice.h"
#include "mail/mailpoolservice.h"


// Defines
#define MAX_TIMEOUT (30*60)


// Define standard used namespaces
using namespace std;
using namespace net;




/**
 * Shows a simple help dialog.
 *
 * @param program_name Program execution name.
 */
void help(string program_name) {
	cout << endl << endl;
	cout << "Help for executed program \"" << program_name << "\" MailSockets!" << endl << endl << endl;
	cout << "This program is a low level C/C++ Linux system implementation of an SMTP" << endl;
	cout << "service for loca use. An example call could look like the following:" << endl;
	cout << "     ./mailserver -d /mail/pool/dir -p 8080" << endl << endl << endl;
	cout << "Parameters:" << endl;
	cout << "     -d  -  Specifies the diretory which should be used as mailpool to" << endl;
	cout << "            store all sent E-Mails there. Nothing will be done if the" << endl;
	cout << "            directory already exists, otherwise it will be created." << endl << endl;
	cout << "     -p  -  Specifies the port on which the server (SMTP Service) will" << endl;
	cout << "            be available. (localhost:port)." << endl << endl;
	cout << "     -o  -  Enables the debug option which writes some server information" << endl;
	cout << "            to the standard output." << endl << endl << endl;
	cout << "Most Common Errors:" << endl;
	cout << "     .) Could not bind address: This error occurs if the given port number" << endl;
	cout << "                                is already occupied." << endl << endl;
}


/**
 * Closes the server socket if it is available.
 */
void dispose_appcontext() {
	if (appcontext::is_initialized()) {
		appcontext::dispose();
	}
}


/**
 * Handles the termination (CTRL-C) signal.
 *
 * @param sig Signal ID.
 */
void int_handler(int sig) {
	dispose_appcontext();
	cout << endl << "Server is shutting down!" << endl;
	exit(EXIT_SUCCESS);
}


/**
 * Registers all used system signal handlers.
 */
void register_signal_handler() {
	signal(SIGINT, int_handler);
}


/**
 * Main program entry point.
 *
 * @param argc Argument count.
 * @param argv Argument array.
 * @return Program exit code.
 */
int main(int argc, char** argv) {
	// Register all used signal handlers
	srand(time(NULL));
	register_signal_handler();


	// Parse Program Parameters
	string program_name = argv[0];
	string directory = "mailpool";
	int port = 6543, c;
	bool debug = false;
	while ((c = getopt(argc, argv, "op:d:")) != EOF) {
		switch (c) {
			case '?':
				fprintf(stderr, "%s error: Unknown parameter\n", program_name.c_str());
				help(program_name);
				exit(1);
				break;
			case 'o':
				debug = true;
				break;
			case 'p':
				port = atoi(optarg);
				break;
			case 'd':
				directory = optarg;
				break;
		}
	}


	// Initializes and sets up the mail pool
	// and try to start the server
	try {
		mailpoolservice mps(directory);
		unique_ptr<loginsystem> lsptr(new ldaplogin());
		//unique_ptr<loginsystem> lsptr(new ldaplogin());
		cout << "Debug Mode: " << (debug ? "On" : "Off") << endl;
		cout << "Listening on localhost:" << port << " using \"" << directory << "\" as SMTP Mail Pool..." << endl;

		// Start server
		appcontext::initialize(port, mps);
		appcontext::get_serversocket()->bind();
		while (true) {
			cout << "Waiting for connections..." << endl;
			net::csocket* connection = appcontext::get_serversocket()->accept();

			// Start smtp mail service and run in own thread
			smtpservice smtps = smtpservice(connection, mps, *lsptr.get());
			smtps.set_debug_mode(debug);
			smtps.set_timeout(MAX_TIMEOUT);
			smtps.start_forked_service();
		}
	} catch(exception& ex) {
		cout << ex.what() << endl;
		help(program_name);
		dispose_appcontext();
		exit(4);
	}

	// Close server socket
	dispose_appcontext();
	return EXIT_SUCCESS;
}
