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

int main (int argc, char **argv) {
  int create_socket;
  char buffer[BUF];
  struct sockaddr_in address;
  int size = 0;
  bool quit = false;
  int options;

  if( argc < 2 ){
     printf("Usage: %s ServerAdresse\n", argv[0]);
     exit(EXIT_FAILURE);
  }

  if ((create_socket = socket (AF_INET, SOCK_STREAM, 0)) == -1)
  {
     perror("Socket error");
     return EXIT_FAILURE;
  }

  memset(&address,0,sizeof(address));
  address.sin_family = AF_INET;
  address.sin_port = htons (PORT);
  inet_aton (argv[1], &address.sin_addr);

  if (connect ( create_socket, (struct sockaddr *) &address, sizeof (address)) == 0)
  {
     printf ("Connection with server (%s) established\n", inet_ntoa (address.sin_addr));
     //size=recv(create_socket,buffer,BUF-1, 0);
     cout << "testitest" << endl;
     if (size>0)
     {
        buffer[size]= '\0';
        printf("%s",buffer);
     }
  }
  else
  {
     perror("Connect error - no server available");
     return EXIT_FAILURE;
  }

char test[10];
  do
  {
    print_options();
    scanf("%d", &options);
    //for some unkown reason it skipes the first fgets
    //so i added a dummy fgets
    fgets(test,10,stdin);
    switch (options) {
      case 1: c_send(create_socket);
              break;
      case 2: c_list(create_socket);
              break;
      case 3: c_read(create_socket);
              break;
      case 4: c_del(create_socket);
              break;
      case 5: c_quit(create_socket);
              quit = true;
              close(create_socket);
              break;
      default: printf("Wrong option please enter a valid number\n");
    }
  }while(!quit);

  return EXIT_SUCCESS;
}
