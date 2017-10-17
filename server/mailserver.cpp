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
#include "filesystem.h"
#include "netutils.h"

#include "net/stream.h"
#include "net/socket.h"
#include "net/serversocket.h"

#include "mail/email.h"
#include "mail/smtpservice.h"
#include "mail/mailpoolservice.h"



// Define standard used namespaces
using namespace std;


// Globally used variables
static net::sserversocket* ss;



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
 * Handles the termination (CTRL-C) signal.
 *
 * @param sig Signal ID.
 */
void int_handler(int sig) {
	ss->close_socket();
	delete ss;
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
		cout << "Debug Mode: " << (debug ? "On" : "Off") << endl;
		cout << "Listening on localhost:" << port << " using \"" << directory << "\" as SMTP Mail Pool..." << endl;

		// Start server
		ss = new net::sserversocket(port);
		while (true) {
			cout << "Waiting for connections..." << endl;
			net::ssocket connection = ss->accept_connection();

			// Start smtp mail service and run in own thread
			smtpservice smtps = smtpservice(connection, mps);
			smtps.set_debug_mode(debug);
			smtps.start_forked_service();
		}
	} catch(exception& ex) {
		cout << ex.what() << endl;
		help(program_name);
		exit(4);
	}

	// Close server socket
	ss->close_socket();
	delete ss;
	return EXIT_SUCCESS;
}

