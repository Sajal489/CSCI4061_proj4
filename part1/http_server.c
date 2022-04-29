#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "http.h"

#define BUFSIZE 4096
#define LISTEN_QUEUE_LEN 5

int keep_going = 1;

void handle_sigint(int signo) {
    keep_going = 0;
}

int main(int argc, char **argv) {
    // First command is directory to serve, second command is port
    if (argc != 3) {
        printf("Usage: %s <directory> <port>\n", argv[0]);
        return 1;
    }
    // Uncomment the lines below to use these definitions:
    const char *serve_dir = argv[1];
     const char *port = argv[2];

    // TODO Complete the rest of this function

    struct sigaction sigact;
    sigact.sa_handler = handle_sigint;
    if (sigfillset(&sigact.sa_mask) == -1) {
        perror("sigfillset");
        return 1;
    }
    sigact.sa_flags = 0;
    if (sigaction(SIGINT, &sigact, NULL) == -1)
    {
        perror("sigaction");
        return 1;
    }
    
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    struct addrinfo *server;

    int ret_val = getaddrinfo(NULL, port, &hints, &server);
    if(ret_val != 0){
        fprintf(stderr, "getaddrinfo failed: %s\n,", gai_strerror(ret_val));
        return 1;
    }
    int sock_fd = 0;
    if((sock_fd = socket(server->ai_family, server->ai_socktype, server->ai_protocol)) == -1){
        perror("socket");
        freeaddrinfo(server);
        return 1;
    }

    if(bind(sock_fd, server->ai_addr, server->ai_addrlen) == -1){
        perror("bind");
        freeaddrinfo(server);
        close(sock_fd);
        return 1;
    }
    freeaddrinfo(server);

    if(listen(sock_fd, LISTEN_QUEUE_LEN) == -1){
        perror("listen");
        close(sock_fd);
        return 1;
    }

    int favi = 0;
    while(keep_going != 0){
        
       if(!favi) printf("Wating for client connection\n");

        int new_socket = accept(sock_fd, NULL, NULL);
        if(new_socket < 0){
            if(errno != EINTR){
            perror("accept connection");
            close(sock_fd);
            return 1;
          }
          else{
              break;
          }
        }

       if(!favi) printf("New client connected\n");
        favi = 0;
        char resource_name[512];
        bzero(resource_name, sizeof(resource_name));

        if(read_http_request(new_socket, resource_name) != 0){
            fprintf(stderr, "read_http_request failed");
            close(new_socket);
            break;
        }
        if(strcmp(resource_name, "/favicon.ico") == 0){
            favi = 1;
            continue;
        }

        char path[512];
        bzero(path, sizeof(path));
        strcat(path, serve_dir);
        if(strcmp(resource_name, "/") == 0) strcat(path, "/index.html");
        else strcat(path, resource_name);


        if(write_http_response(new_socket, path) != 0){
            fprintf(stderr, "write_http_request failed");
            close(new_socket);
            break;
        }

    }

  if (close(sock_fd) == -1) {
        perror("close");
        return 1;
    }
    return 0;
}
