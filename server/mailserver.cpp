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

// Include custom classes
#include "filesystem.h"
#include "netutils.h"

#include "net/stream.h"
#include "net/socket.h"
#include "net/serversocket.h"

#include "mail/email.h"
#include "mail/smtpservice.h"
#include "mail/mailpoolservice.h"


//Defines
#define BUF 1024



// Define standard used namespaces
using namespace std;


// Globally used variables
static net::sserversocket* ss;



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
	register_signal_handler();

	// Parse Program Parameters
	string program_name = argv[0];
	string directory = "./mailpool/";
	int port = 6543, c;
	while ((c = getopt(argc, argv, "p:d:")) != EOF) {
		switch (c) {
			case '?':
				fprintf(stderr, "%s error: Unknown parameter\n", program_name.c_str());
				exit(1);
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
	mailpoolservice mps = mailpoolservice(directory);
	cout << "Listening on localhost:" << port << " using \"" << directory << "\" as SMTP Mail Pool..." << endl;

	// Start server
	try {
		ss = new net::sserversocket(port);
		while (true) {
			cout << "Waiting for connections..." << endl;
			net::ssocket connection = ss->accept_connection();

			// Start smtp mail service and run in own thread
			smtpservice smtps = smtpservice(connection);
			smtps.start_forked_service();
		}
	} catch(exception& ex) {
		cout << ex.what() << endl;
		exit(4);
	}

	// Close server socket
	ss->close_socket();
	delete ss;
	return EXIT_SUCCESS;
}

