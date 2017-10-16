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
  char message[BUF-200] = "";
  char buffer[BUF] = "SEND\n";
  char OK[5] = "";

  do {
    printf("Sender(max8)\n");
    fgets(sender, 8, stdin);
  } while(strlen(sender) > 8);
  strcat(buffer,sender);

  do{
    printf("Receiver(max 8)\n");
    fgets(receiver, 8, stdin);
  }while(strlen(receiver) > 8);
  strcat(buffer,receiver);

  do{
    printf("subject(max 80)\n");
    fgets(subject, 80, stdin);
  }while(strlen(subject) > 80);
  strcat(buffer,subject);

  printf("message (ends with newline.newline)\n");
  do{
    fgets(message, BUF, stdin);
    strcat(buffer,message);
  }while(strcmp(message,".\n"));
  printf("%s\n", buffer);
  write(create_socket,buffer,strlen(buffer));

  read(create_socket, OK,5);
  if(strcmp(OK, "OK"))
  {
    printf("successfull\n");
  }
  else
  {
    printf("unsuccessfull\n");
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
  int size;
  int n = 0;
  char amount[10] = "";
  char username[9] = "";
  char buffer[BUF] = "";
  char tosend[20] = "LIST\n";

  do{
    printf("Please enter the username(max 8 zeichen)\n");
    fgets(username, 8, stdin);
  }while(strlen(username) > 8);

  strcat(tosend,username);
  send(create_socket, tosend, strlen(tosend),0);

  readline(amount,create_socket,10);
  n = atoi(amount);
  printf("%d messages\n", n);

  for(int i = 0;i < n; i++)
  {
    readline(buffer,create_socket,BUF);
    printf("%s\n", buffer);
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
  int size;
  char username[9]= "";
  char number[10] = "";
  char buffer[BUF] = "";
  char tosend[30] = "READ\n";
  char OK[10] = "";

  do{
    printf("Please enter the username(max 8)\n");
    fgets(username, 8, stdin);
  }while(strlen(username)> 8);
  strcat(tosend,username);

  printf("What message do you want to read\n");
  fgets(number, 8, stdin);
  strcat(tosend, number);

  send(create_socket, tosend, strlen(tosend), 0);

  readline(OK, create_socket, 10);

  if(!strcmp(OK,"OK"))
  {
    printf("Error\n");
    return;
  }

  read(create_socket,buffer,BUF-1);
  printf("%s\n", buffer);
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
    printf("Please enter the username\n");
    fgets(username, 8, stdin);
  }while(strlen(username) > 8);
  strcat(tosend,username);

  printf("What message do you want to delete\n");
  fgets(number, 8, stdin);
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
    printf("Delete successfull\n");
  }
  else
  {
    printf("Delete unsuccessfull\n");
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
    cout << "What do you wanne do?" << endl;
    cout << "1. Send message" << endl;
    cout << "2. List of all Messages you send" << endl;
    cout << "3. Read a spezific Message" << endl;
    cout << "4. Delete a spezific Message" << endl;
    cout << "5. Quit" << endl;
}
