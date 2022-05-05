#include <stdlib.h>
#include <stdio.h> 
#include <string.h>         //required by memset and by custom function to check errors
#include <strings.h>
#include <sys/socket.h>
#include <sys/un.h>         // must be related to unix sockets
#include <unistd.h>

char buffer[4096];

// function to create a socket as AF_UNIX or type SOCK_STREAM

int SocketCreate(void) {
  int unix_socket;
  unix_socket = socket(AF_UNIX , SOCK_STREAM , 0);
  return unix_socket;
}


// functions to avoid checking the return code of socket actions in main code

void CheckReturnCodeCreateSocket(int return_code) {
    if (return_code == -1) {
        printf("FAIL : return code is %d for Create Socket\n", return_code);
        exit(1);
    }
    else {
        printf("SUCCESS : return code is %d for Create Socket\n", return_code);
    }
}

void CheckReturnCodeBindSocket(int return_code) {
    if (return_code == -1) {
        printf("FAIL : return code is %d for Bind Socket\n", return_code);
        exit(1);
    }
    else {
        printf("SUCCESS : return code is %d for Bind Socket\n", return_code);
    }
}

void CheckReturnCodeListenSocket(int return_code) {
    if (return_code == -1) {
        printf("FAIL : return code is %d for Listen Socket\n", return_code);
        exit(1);
    }
    if (return_code == 0) {
        printf("SUCCESS : return code is %d for Listen Socket\n", return_code);
    }    
}

void CheckReturnCodeAcceptSocket(int return_code) {
    if (return_code < 0) {
        printf("FAIL : return code is %d for Accept Socket\n", return_code);
        exit(1);
    }
    else {
        printf("SUCCESS : return code is %d for Accept Socket\n", return_code);
    } 
}

void CheckReturnCodeReadSocket(int return_code) {
    if (return_code < 0) {
        printf("FAIL : return code is %d for Read Socket\n", return_code);
        exit(1);
    }
    else {
        printf("SUCCESS : return code is %d for Read Socket\n", return_code);
    } 
}


int main() {

  // create socket and check return code
  int unix_socket = SocketCreate();
  CheckReturnCodeCreateSocket(unix_socket);
  
  // socket parameters, here we use an existing struct (sockaddr_un) from netinet/un.h
  struct sockaddr_un unix_socket_params;
  unix_socket_params.sun_family = AF_UNIX;
  char socket_name[22] = "/tmp/unix_socket.sock";
  strncpy(unix_socket_params.sun_path, socket_name, sizeof(unix_socket_params.sun_path) -1);
  
  // here we bind the socket (sockaddr_un* can be cast to type sockaddr*)
  int bind_unix_socket = bind(unix_socket, (const struct sockaddr *) &unix_socket_params, sizeof(unix_socket_params));
  CheckReturnCodeBindSocket(bind_unix_socket);
  
  // start listening on the socket (5 is the backlog of how many connection can queue up before you accept() them
  int listen_socket = listen(unix_socket,5);
  CheckReturnCodeListenSocket(listen_socket);

  // accept new connection
  int connected_socket = accept(unix_socket, (struct sockaddr *)NULL, NULL);
  CheckReturnCodeAcceptSocket(connected_socket);  

  // read from socket
  char greetings[] = "ask your question : ";
  send(connected_socket, greetings, strlen(greetings), 0);
  int socket_reader = read(connected_socket, buffer, 4096);
  CheckReturnCodeReadSocket(socket_reader);
  char encryption[13] = "encryption?\n";
  printf("buffer : ##%s##", buffer);
  printf("encryption : ##%s##", encryption);
  if (strcmp(buffer,encryption) == 0) { 
    char server_answer[19] = "server answer : 1\n";
    send(connected_socket, server_answer, strlen(server_answer), 0);
  }
  else {
    char server_answer[34] = "server answer : invalid question\n";
    send(connected_socket, server_answer, strlen(server_answer), 0);
  }
  unlink(socket_name) ;
}
