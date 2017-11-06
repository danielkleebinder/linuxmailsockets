// Include C++/11 Headers
#include <string>
#include <iostream>
#include <thread>
#include <sstream>
#include <stack>

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
#include <termios.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

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


int c_login(int create_socket)
{
  char username[20];
  char password[BUF];
  char response[5];
  int trys = 1;
  static struct termios oldt, newt;

  /*saving the old settings of STDIN_FILENO and copy settings for resetting*/
  tcgetattr( STDIN_FILENO, &oldt);
  newt = oldt;

  /*setting the approriate bit in the termios struct*/
  newt.c_lflag &= ~(ECHO);

  do {
    char buffer[BUF] = "LOGIN\n";
    do {
      printf("What is your Username\n");
      fgets(username, 20, stdin);
      fflush(stdin);
    } while(strlen(username)> 9);

    /*setting the new bits*/
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);

      do {
        printf("What is your password\n");
        fgets(password, BUF, stdin);
        fflush(stdin);
      } while(!(strlen(password) > 0));
      password[strlen(password)-1] = '\0';

      /*resetting our old STDIN_FILENO*/
      tcsetattr( STDIN_FILENO, TCSANOW, &oldt);

      strcat(buffer,username);
      strcat(buffer,password);

      write(create_socket,buffer,strlen(buffer)+1);

      read(create_socket, response,5);
      printf("response: %s\n", response);
      trys++;
      if(!((response[0] = 'O') && (response[1] == 'K')))
      {
        printf("An error accured plz try again\n");
      }

      if(trys > 3)
      {
        printf("Maximum numbers of trys, plz try again later\n");
        return 0;
      }
  } while(!((response[0] = 'O') && (response[1] == 'K')) && trys < 4);
  return 1;
}

void c_logout(int create_socket)
{
  send(create_socket, "LOGOUT\n", 7, 0);
  //c_quit(create_socket);
}

/*
*/

int c_inputattachment(int create_socket,stack<char*> &stk)
{
  FILE *fptr;
  char input[BUF];
  int i = 0;
  printf("Do you want to add attachments? y/n \n");
  fgets(input, 10, stdin);
  fflush(stdin);
  if(!(input[0] == 'y' && input[1] == '\n'))
  {
    printf("No attachments are getting added\n");
    return 0;
  }

  do {
    printf("Filename(relativer pfad)? Quit with Q\n");
    fgets(input,BUF,stdin);
    fflush(stdin);

    if(input[0] == 'Q')
    {
      break;
    }

    input[strlen(input)-1] = '\0';
    if((fptr = fopen(input,"rb")) == NULL)
    {
      printf("An error accured(mby the wrong filename?)\n");
      continue;
    }
    else
    {
      i++;
      char * test = (char*) malloc(sizeof(char)*(2048+1));
      strncpy(test,input,2048);
      stk.push(test);
    }
  } while(!(input[0] == 'Q'));
  return i;
}

void c_sendattachment(int create_socket,stack<char*> &stk)
{
  char* filename;
  uint64_t filesize;
  //char filesize[BUF] = "";
  struct stat filestat;
  //geting current working directory
  char * cwd;
  cwd = get_current_dir_name();
  DIR *current = opendir(cwd);

  while(!(stk.empty()))
  {
    //getting filename from the stack
    filename = stk.top();
    stk.pop();
    if(filename == NULL)
    {
      printf("break trough filename\n");
      break;
    }
    //sending filename
    printf("filename: %s\n", filename);
    write(create_socket,filename,strlen(filename)+1);
    //getting filesize
    if((fstatat(dirfd(current),filename,&filestat,0)) < 0)
    {
      printf("an error accured for the file %s\n", filename);
      int errsv = errno;
      printf("errorcode was: %d\n", errsv);
      //write(create_socket,filename,strlen(filename)+1);
      continue;
    }
    filesize = filestat.st_size;
    printf("%ld\n", filesize);
    write(create_socket,&filesize,8);

    FILE *fp;
    fp = fopen(filename,"rb");
    for(uint64_t  i = 0; i < filesize; i++)
    {
      uint8_t data = 0;
      fread(&data,sizeof(uint8_t),1,fp);
      write(create_socket,&data,1);
    }
    fclose(fp);
  }
  free(cwd);
}

void c_saveattachments(int create_socket)
{
    //int size;
    char number[BUF] = "";
    char tosend[BUF] = "ATT\n";
    char OK[10] = "";
    uint8_t numofatt;
    char filename[BUF];
    uint64_t filesize;
    uint8_t data;
    //char * cwd;
    //cwd = get_current_dir_name();
    //DIR *current = opendir(cwd);


    printf("Enter the message number you want to save the attacments from: ");
    fgets(number, 8, stdin);
    fflush(stdin);
    strcat(tosend, number);

    send(create_socket, tosend, strlen(tosend), 0);

    readline(OK, create_socket, 10);

    if(!((OK[0] = 'O') && (OK[1] == 'K')))
    {
      printf("Error\n");
      return;
    }

    read(create_socket,&numofatt,1);

    if(numofatt <= 0)
    {
      printf("no attachments \n");
      return;
    }

    for(uint8_t i = 0; i < numofatt; i++)
    {
      readline(filename,create_socket,BUF);
      printf("filename: %s\n", filename);
      //if(filename[strlen(filename)-1] != '\n')
      //{
        filename[strlen(filename)-1] = '\0';
      //}
      
      read(create_socket,&filesize,8);
      printf("filesize: %ld\n", filesize);

      FILE * fp = fopen(filename,"wb");
      for(uint64_t j = 0; j < filesize; j++)
      {
        read(create_socket,&data,1);
        fwrite(&data,1,1,fp);
      }
      fclose(fp);
    }
    //free(cwd);
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
  char receiver[BUF] = "";
  char subject[BUF] = "";
  char message[BUF] = "";
  char buffer[BUF] = "SEND\n";
  char OK[5] = "";
  uint8_t NumOfAtt = 0;
  stack<char*> stk;

  do{
    printf("Receiver (Max 8 Characters): ");
    fgets(receiver, BUF, stdin);
    fflush(stdin);
  }while(strlen(receiver) > 9);
  strcat(buffer,receiver);

  do{
    printf("Subject (Max 80 Characters): ");
    fgets(subject, BUF, stdin);
    fflush(stdin);
  }while(strlen(subject) > 80);
  strcat(buffer,subject);

  write(create_socket,buffer,strlen(buffer));

  printf("Enter a Message (Ends with newline.newline (\\n.\\n)):\n");
  do{
    fgets(message, BUF, stdin);
    write(create_socket,message,strlen(message));
  }while(strcmp(message,".\n"));

  NumOfAtt = c_inputattachment(create_socket,stk);
  printf("number of attachments: %d\n", NumOfAtt);
  //sprintf(STRNOA,"%d",NumOfAtt);
  //write(create_socket,STRNOA,strlen(STRNOA)+1);
  write(create_socket,&NumOfAtt,1);

  if(NumOfAtt > 0)
  {
    c_sendattachment(create_socket,stk);
  }

  read(create_socket, OK,5);

  if((OK[0] = 'O') && (OK[1] == 'K'))
  {
    printf("Sent mail successfully!\n");
  }
  else
  {
    printf("An error occurred, please try again later!\n");
  }
  printf("\n\n");
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
  char buffer[BUF] = "";
  char tosend[BUF] = "LIST\n";

  send(create_socket, tosend, strlen(tosend),0);

  readline(amount,create_socket,10);
  n = atoi(amount);
  printf("%d message(s) available\n", n);

  for(int i = 0;i < n; i++)
  {
    readline(buffer,create_socket,BUF);
    printf(" %d.) %s\n", i+1,buffer);
  }
  printf("\n\n");
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
  char number[BUF] = "";
  char buffer[BUF] = "";
  char tosend[BUF] = "READ\n";
  char OK[10] = "";

  printf("Enter the message number you want to read: ");
  fgets(number, 8, stdin);
  fflush(stdin);
  strcat(tosend, number);

  send(create_socket, tosend, strlen(tosend), 0);

  readline(OK, create_socket, 10);

  if(!((OK[0] = 'O') && (OK[1] == 'K')))
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
  printf("Do you want to save the attachments? y/n\n");
  char input[10];
  fgets(input,10,stdin);
  fflush(stdin);
  if(input[0] == 'y')
  {
    c_saveattachments(create_socket);
  }
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
  char number[BUF];
  char buffer[BUF];
  char tosend[30] = "DEL\n";

  printf("Enter the message number you want to delete: ");
  fgets(number, BUF, stdin);
  fflush(stdin);
  strcat(tosend,number);
  send(create_socket, tosend,strlen(tosend),0);

  size = read(create_socket, buffer, BUF-1);

  if(size <= 0)
  {
    printf("error\n");
    return;
  }

  if((buffer[0] = 'O') && (buffer[1] == 'K'))
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
  printf(" 5. Logout\n");
  printf(" 6. Quit\n");
}
