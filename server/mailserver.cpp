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
#include <thread>

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
#include "email.h"
#include "smtphandler.h"
#include "filesystem.h"
#include "netutils.h"
#include "net/stream.h"
#include "net/socket.h"
#include "net/serversocket.h"


//Defines
#define BUF 1024
#define ARCHIVE "ARCHIVE_D"



// Define standard used namespaces
using namespace std;
using namespace netutils;
using namespace fs;



/**
 * Handles the connection from a client.
 *
 * @param socket Connected client socket.
 */
void connected(net::socket& connection) {
	smtphandler handler(connection);

	stream socket_stream = connection.get_stream();

	string line;
	do {
		// Read line by line using the netutils utility namespace
		line = socket_stream.sreadline();

		// Check if line is null or empty
		if (line.empty()) {
			continue;
		}

		// Start correct mail protocol
		if (line == "SEND") {
			handler.send();
		}

		if (line == "LIST") {
			handler.list();
		}

		if (line == "READ") {
			handler.read();
		}

		if (line == "DEL") {
			handler.del();
		}

		// Print received message
		cout << "Message Received: " << line << endl;
	} while (line != "quit");

	// Closing the socket
	handler.quit();
	cout << "Closing socket ID-" << connection.get_handler_id() << endl;
	connection.close_socket();
}



/**
 * Handles the termination (CTRL-C) signal.
 *
 * @param sig Signal ID.
 */
void int_handler(int sig) {
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
 * Initializes the mailpool and all used directories.
 *
 * @param directory Mailpool directory.
 */
void initialize_mailpool(string directory) {
	if (directory.back() != '/') {
		directory.back() = '/';
	}

	// Create directories
	if (!fs::exists(directory)) {
		if (!fs::make_dir(directory)) {
			fprintf(stderr, "Error: Can not create mail pool directory!");
			exit(2);
		}
		if (!fs::make_dir(directory + ARCHIVE)) {
			fprintf(stderr, "Error: Can not create archive directory!");
			exit(2);
		}
	}
}




/**
 * Main program entry point.
 *
 * @param argc Argument count.
 * @param argv Argument array.
 * @return Program exit code.
 */
int main(int argc, char** argv) {
	// Test for email class
	email mail;
	mail.set_sender("Daniel");
	mail.set_receiver("Thomas");
	mail.set_subject("First E-Mail Implementation");
	mail.set_message("My message\n.\n");


	// Register signal handler
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
	initialize_mailpool(directory);
	cout << "Listening on localhost:" << port << " using \"" << directory << "\" as SMTP Mail Pool..." << endl;

	// Start server
	net::serversocket ss = net::serversocket(port);
	while (true) {
		cout << "Waiting for connections..." << endl;
		net::socket connection = ss.accept();

		// Start connection thread
		thread con_thread(connected, std::ref(connection));
		con_thread.detach();
	}

	// Close server socket
	return EXIT_SUCCESS;
}

