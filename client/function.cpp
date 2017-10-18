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

/*
function readline
  @param:
    char* buffer: pointer to a chararray
          whitch is to be modified
    create_socket: the network socket
    max: the maximum on aviable space in the char array
reads the socket one char at a time until a newline
and saves it in a chararray
*/
void readline(char* buffer,int create_socket,int max)
{
  char c;
  int size = 0;
  int i = 0;

  do{
    if(i < max)
    {
      size = read(create_socket, &c, 1);
      buffer[i] = c;
    }
    else
    {
      printf("error while reading socket, message was to long\n");
      return;
    }
    if(size <= 0)
    {
      printf("error while reading socket\n");
      return;
    }
    i++;
  }while(c != '\n');
  buffer[i] = '\0';
}


/*
function c_send
sends the message to the server
sends it in one big string
server returns ok if successfull
@param: the socket
*/
void c_send(int create_socket)
{
  char sender[9] = "";
  char receiver[9] = "";
  char subject[81] = "";
  char message[BUF] = "";
  char buffer[BUF] = "SEND\n";
  char OK[5] = "";

  do {
    printf("Sender (Max 8 Characters): ");
    fgets(sender, 8, stdin);
    fflush(stdin);
  } while(strlen(sender) > 8);
  strcat(buffer,sender);

  do{
    printf("Receiver (Max 8 Characters): ");
    fgets(receiver, 8, stdin);
    fflush(stdin);
  }while(strlen(receiver) > 8);
  strcat(buffer,receiver);

  do{
    printf("Subject (Max 80 Characters): ");
    fgets(subject, 80, stdin);
    fflush(stdin);
  }while(strlen(subject) > 80);
  strcat(buffer,subject);

  write(create_socket,buffer,strlen(buffer));

  printf("Enter a Message (Ends with newline.newline (\\n.\\n)):\n");
  do{
    fgets(message, BUF, stdin);
    write(create_socket,message,strlen(message));
  }while(strcmp(message,".\n"));

  read(create_socket, OK,5);
  if(strcmp(OK, "OK"))
  {
    printf("Sent mail successfully!\n");
  }
  else
  {
    printf("An error occurred, please try again later!\n");
  }
}

/*
function c_list
lists all messanges for a specific user
first sends the username to the server
the server sends the number of messanges and
all messanges one after another
@param: the socket
*/
void c_list(int create_socket)
{
  //int size;
  int n = 0;
  char amount[10] = "";
  char username[9] = "";
  char buffer[BUF] = "";
  char tosend[20] = "LIST\n";

  do{
    printf("Please enter a username (Max 8 Characters): ");
    fgets(username, 8, stdin);
    fflush(stdin);
  }while(strlen(username) > 8);

  strcat(tosend,username);
  send(create_socket, tosend, strlen(tosend),0);

  readline(amount,create_socket,10);
  n = atoi(amount);
  printf("%d message(s) available\n", n);

  for(int i = 0;i < n; i++)
  {
    readline(buffer,create_socket,BUF);
    printf(" %d.) %s\n", i+1,buffer);
  }
}

/*
function c_read
reads a specific message for a user
sends the username and the message to be read to the server
server sends ok if the message is there and the message as one big string

@param: the socket
*/
void c_read(int create_socket)
{
  //int size;
  char username[9]= "";
  char number[10] = "";
  char buffer[BUF] = "";
  char tosend[30] = "READ\n";
  char OK[10] = "";

  do{
    printf("Please enter a username(Max 8 Characters): ");
    fgets(username, 8, stdin);
    fflush(stdin);
  }while(strlen(username)> 8);
  strcat(tosend,username);

  printf("Enter the message number you want to read: ");
  fgets(number, 8, stdin);
  fflush(stdin);
  strcat(tosend, number);

  send(create_socket, tosend, strlen(tosend), 0);

  readline(OK, create_socket, 10);

  if(!strcmp(OK,"OK"))
  {
    printf("Error\n");
    return;
  }


  printf("Message:\n");
  while(strcmp(buffer, ".\n") != 0)
  {
    readline(buffer,create_socket,BUF);
    printf("%s", buffer);
  }
  read(create_socket,buffer,BUF-1); //get rid of everything still in the stream
  printf("\n\n");
}

/*
function c_del
deletes a specific message for a user
sends the username and the message to be deleted to the server
server sends ok if the message was deleted
@param: the socket
*/
void c_del(int create_socket)
{
  int size;
  char username[9];
  char number[10];
  char buffer[BUF];
  char tosend[30] = "DEL\n";

  do{
    printf("Please enter a username (Max 8 Characters): ");
    fgets(username, 8, stdin);
    fflush(stdin);
  }while(strlen(username) > 8);
  strcat(tosend,username);

  printf("Enter the message number yo want to delete: ");
  fgets(number, 8, stdin);
  fflush(stdin);
  strcat(tosend,number);
  send(create_socket, tosend,strlen(tosend),0);

  size = read(create_socket, buffer, BUF-1);

  if(size <= 0)
  {
    printf("error\n");
    return;
  }

  if(strcmp(buffer, "OK\n"))
  {
    printf("Successfully deleted the message!\n");
  }
  else
  {
    printf("An error occurred while deleting the message, please try again later!\n");
  }

}

//quits the connection
void c_quit(int create_socket)
{
  send(create_socket, "quit\n", 7, 0);
}

//prints the options
void print_options()
{
  printf("What do you want to do?\n");
  printf("Select via typing the spezific number\n");
  printf(" 1. Send message\n");
  printf(" 2. List of all Messages you received\n");
  printf(" 3. Read a spezific Message\n");
  printf(" 4. Delete a spezific Message\n");
  printf(" 5. Quit\n");
}
