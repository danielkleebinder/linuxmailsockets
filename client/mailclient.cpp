// Include C++/11 Headers
#include <string>
#include <iostream>
#include <thread>
#include <sstream>

/* myclient.c */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "function.h"
#define BUF 2048
#define PORT 6543

using namespace std;


void help(char* program_name) {
	printf("Usage: %s ServerAddress [-p port]\n\n", program_name);
	printf("This client is by default running on port 6543. To change\n");
    printf("this, use the [-p port] option.\n\n");
}



int main (int argc, char **argv) {
  int create_socket;
  char buffer[BUF];
  char * username;
  struct sockaddr_in address;
  int size = 0;
  bool quit = false;
  int options;
  int port = 6543;
  int c;
  bool loggedin = false;

  char* program_name = argv[0];
  while ((c = getopt(argc, argv, "p:")) != EOF) {
    switch (c) {
      case '?':
		help(program_name);
        exit(1);
        break;
      case 'p':
        port = atoi(optarg);
        break;
    }
  }
  if( argc < 2 ){
	 help(program_name);
     exit(EXIT_FAILURE);
  }

  if ((create_socket = socket (AF_INET, SOCK_STREAM, 0)) == -1)
  {
	 help(program_name);
     perror("Socket error");
     return EXIT_FAILURE;
  }

  memset(&address,0,sizeof(address));
  address.sin_family = AF_INET;
  address.sin_port = htons (port);
  inet_aton (argv[1], &address.sin_addr);

  if (connect ( create_socket, (struct sockaddr *) &address, sizeof (address)) == 0)
  {
     printf ("Connection with server (%s) established\n", inet_ntoa (address.sin_addr));
     //size=recv(create_socket,buffer,BUF-1, 0);
     if (size>0)
     {
        buffer[size]= '\0';
        printf("%s",buffer);
     }
  }


  else
  {
     perror("Connect error - no server available");
	 help(program_name);
     return EXIT_FAILURE;
  }

  char connectionok[5];
  readline(connectionok,create_socket,5);
  if(!((connectionok[0] == 'O') & (connectionok[1] == 'K')))
  {
    printf("Error While Connecting!\nThis error may occur, if the server has temporarily blocked this IP address.\n\n");
    close(create_socket);
	help(program_name);
    return EXIT_FAILURE;
  }



  char test[10];
  do
  {
    //LOGIN
    if(loggedin == false)
    {
      while((username = c_login(create_socket)) == NULL)
      {
        char input[10];
        printf("do you want to quit? y/n\n" );
        fgets(input,10,stdin);
        fflush(stdin);

        if((input[0] == 'y') & (input[1] == '\n'))
        {
          c_quit(create_socket);
          close(create_socket);
          return EXIT_SUCCESS;
        }
      }
      loggedin = true;
    }

    printf("You are logged in as %s", username);
    print_options();
    printf("Option > ");
    fgets(test,10,stdin);
    options = atoi(test);
    fflush(stdin); //flushing user input if he overshoots or something

    //scanf("%d", &options);
    //for some unkown reason it skipes the first fgets
    //so i added a dummy fgets
    //fgets(test,10,stdin);
    //the problem was that scanf leaves a newline in the buffer and the next
    //fgets reads the newline
    switch (options) {
      case 1: c_send(create_socket);
              break;
      case 2: c_list(create_socket);
              break;
      case 3: c_read(create_socket);
              break;
      case 4: c_del(create_socket);
              break;
      case 5: c_logout(create_socket);
              loggedin = false;
              break;
      case 6: c_quit(create_socket);
              quit = true;
              close(create_socket);
              break;
      default: printf("\n\nWrong option! Please enter a valid number between 1 and 6!\n\n");
    }
  }while(!quit);
  free(username);
  return EXIT_SUCCESS;
}
