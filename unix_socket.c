#include <stdlib.h>
#include <stdio.h> 
#include <string.h>         //required by memset and by custom function to check errors
#include <strings.h>
#include <sys/socket.h>
#include <sys/un.h>         // must be related to unix sockets
#include <unistd.h>
#include <poll.h>           //needed to poll connections

char buffer[4096];
struct pollfd poll_file_descriptors[20];
int poll_struct_number = 0;
int poll_struct_file_descriptors_total = 20;
int poll_timeout = 2;

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

void CheckReturnCodePoller(int return_code, int pooler_num) {
    if (return_code < 0) {
        printf("FAIL : return code is %d for poll call\n", return_code);
        exit(1);
    }
    if (return_code > 0) {
        printf("SUCCESS : return code for poll call is %d (number of POOLIN events found), checking pooler #%d\n", return_code, pooler_num);
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

    while(1) {
        // loop in all the pollfds structs and set POLLIN or POLLPRI as an event of interest (poll will try to match revents in the same struct against these events)
        for (poll_struct_number = 0; poll_struct_number < poll_struct_file_descriptors_total; poll_struct_number++) {
            
            // fill the pollfd structs using unix_socket as our fd
            poll_file_descriptors[poll_struct_number].fd = unix_socket;
            poll_file_descriptors[poll_struct_number].events = POLLIN;
                
            // poll all fd's
            int poller = poll(poll_file_descriptors, 1, poll_timeout);
            CheckReturnCodePoller(poller, poll_struct_number);


            if (poll_file_descriptors[poll_struct_number].revents == POLLIN) {
                // launch accept() if we got a POOLIN event
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
            }
        }
    }
    unlink(socket_name) ;
}
