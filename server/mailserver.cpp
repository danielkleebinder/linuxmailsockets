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
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Include custom classes
#include "email.h"


// Defines
#define BUF 1024


// Define standard used namespaces
using namespace std;


/**
 * Main program entry point.
 *
 * @param argc Argument count.
 * @param argv Argument array.
 * @return Program exit code.
 */
int main (int argc, char** argv) {
	int create_socket, new_socket;
	socklen_t addrlen;
	char buffer[BUF];
	int size, c;
	struct sockaddr_in address, cliaddress;

	// Test for email class
	email mail;
	mail.set_sender("Daniel");
	mail.set_receiver("Thomas");
	mail.set_subject("First E-Mail Implementation");
	mail.set_message("My message\n.\n");


	// Parse Program Parameters
	string program_name = argv[0];
	string directory = "./mailpool/";
	int port = 6543;

	while ((c = getopt(argc, argv, "p:d:")) != EOF) {
		switch (c) {
			case '?':
				fprintf(stderr, "%s error: Unknown parameter", program_name.c_str());
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


	// Create the socket
	create_socket = socket(AF_INET, SOCK_STREAM, 0);

	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	// Bind the socket
	if (bind (create_socket, (struct sockaddr *) &address, sizeof (address)) != 0) {
		perror("bind error");
		return EXIT_FAILURE;
	}
	listen(create_socket, 5);
	addrlen = sizeof (struct sockaddr_in);

	cout << "Listening on localhost:" << port << " using \"" << directory << "\" as SMTP Mail Pool..." << endl;

	// Start server
	while (true) {
		 cout << "Waiting for connections..." << endl;
		 new_socket = accept(create_socket, (struct sockaddr *) &cliaddress, &addrlen);
		 if (new_socket > 0) {
			printf("Client connected from %s:%d...\n", inet_ntoa(cliaddress.sin_addr), ntohs(cliaddress.sin_port));
			strcpy(buffer,"Welcome to myserver, Please enter your command:\n");
			send(new_socket, buffer, strlen(buffer),0);
		 }
		 do {
			size = recv (new_socket, buffer, BUF - 1, 0);
			if(size > 0) {
				 buffer[size] = '\0';
				 printf ("Message received: %s\n", buffer);
			} else if (size == 0) {
				 printf("Client closed remote socket\n");
				 break;
			} else {
				 perror("recv error");
				 return EXIT_FAILURE;
			}
		 } while (strncmp(buffer, "quit", 4) != 0);
		 close(new_socket);
	}
	close(create_socket);
	return EXIT_SUCCESS;
}
