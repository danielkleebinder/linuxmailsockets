#ifndef FUNCTION_H_INCLUDED
#define FUNCTION_H_INCLUDED

void readline(char* buffer,int create_socket,int max);
void c_send(int create_socket);
void c_list(int create_socket);
void c_read(int create_socket);
void c_del(int create_socket);
void c_quit(int create_socket);
char* c_login(int create_socket);
void c_logout(int create_socket);
int c_inputattachment(int create_socket);
void print_options();



#endif
