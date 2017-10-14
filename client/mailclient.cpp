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
#define BUF 1024
#define PORT 6543

using namespace std;

void c_send(int create_socket)
{
  char sender[9];
  char receiver[9];
  char subject[81];
  char message[BUF];
  //char test[10];

  send(create_socket, "SEND\n", 6, 0);

  do {
    std::cout << "Sender(max 8)" << endl;
    fgets(sender, 8, stdin);
  } while(strlen(sender) > 8);

  send(create_socket, sender, strlen (sender), 0);

  std::cout << "Receiver(max 8)" << endl;
  fgets(receiver, 8, stdin);
  send(create_socket, receiver, strlen (receiver), 0);

  std::cout << "subject(max 80)" << endl;
  fgets(subject, 80, stdin);
  send(create_socket, subject, strlen (subject), 0);

  std::cout << "message" << endl;
  fgets(message, BUF, stdin);
  send(create_socket, message, strlen (message), 0);
  send(create_socket, ".\n", 3, 0);
}

void c_list(int create_socket)
{
  int size;
  int n = 0;
  char amount[10];
  char username[9];
  char buffer[BUF];
  send(create_socket, "LIST\n", 6, 0);
  std::cout << "Please enter the username" << endl;
  fgets(username, 8, stdin);
  send(create_socket, username, strlen(username), 0);

  size = read(create_socket, amount, 1);
  n = atoi(amount);
  cout << n << " messages" << endl;

  if(size <= 0)
    return;

  for(int i = 0; i <= n; i++)
  {
      size = read(create_socket, buffer, BUF-1);
      if(size == 0)
      {
        cout << "no remaining messages";
      }
      else if(size == -1)
      {
        cout << "error";
        return;
      }
      else
      {
        cout << buffer << endl;
      }
  }

}

void c_read(int create_socket)
{
  int size;
  char username[9];
  char number[10];
  char buffer[BUF];
  send(create_socket, "READ\n", 6, 0);
  std::cout << "Please enter the username" << endl;
  fgets(username, 8, stdin);
  send(create_socket, username, strlen(username), 0);

  cout << "What message do you want to read" << endl;
  fgets(number, 8, stdin);
  send(create_socket, number, strlen(number), 0);

      size = read(create_socket, buffer, BUF-1);
      if(size == 0)
      {
        cout << "no remaining messages";
      }
      else if(size == -1)
      {
        cout << "error";
        return;
      }
      else
      {
        cout << buffer << endl;
      }

}

void c_del(int create_socket)
{
  int size;
  char username[9];
  char number[10];
  char buffer[BUF];
  send(create_socket, "DEL\n", 5, 0);
  std::cout << "Please enter the username" << endl;
  fgets(username, 8, stdin);
  send(create_socket, username, strlen(username), 0);

  cout << "What message do you want to delete" << endl;
  fgets(number, 8, stdin);
  send(create_socket, number, strlen(number), 0);

  size = read(create_socket, buffer, BUF-1);

  if(strcmp(buffer, "OK"))
  {
    cout << "Delete successfull";
  }
  else
  {
    cout << "Delete unsuccessfull";
  }

}

void c_quit(int create_socket)
{
  send(create_socket, "quit\n", 7, 0);
}

void print_options()
{
    cout << "What do you wanne do?" << endl;
    cout << "1. Send message" << endl;
    cout << "2. List of all Messages you send" << endl;
    cout << "3. Read a spezific Message" << endl;
    cout << "4. Delete a spezific Message" << endl;
    cout << "5. Quit" << endl;
}


int main (int argc, char **argv) {
  int create_socket;
  char buffer[BUF];
  struct sockaddr_in address;
  int size;
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
     size=recv(create_socket,buffer,BUF-1, 0);
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
    std::cin >> options;
    cin.clear();

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
      default: cout << "Wrong option please enter a valid number" << endl;
    }
  }while(!quit);
  /*
  do {
     printf ("Send message: ");
     fgets (buffer, BUF, stdin);
     send(create_socket, buffer, strlen (buffer), 0);
  }
  */
  return EXIT_SUCCESS;
}
