#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "connection_queue.h"
#include "http.h"

#define BUFSIZE 512
#define LISTEN_QUEUE_LEN 5
#define N_THREADS 12

int keep_going = 1;
char *serve_dir;
connection_queue_t queue;

void handle_sigint(int signo)
{
    connection_queue_shutdown(&queue);
    keep_going = 0;
}
void *thread_func(void *arg)
{
    char *server_dir = (char *) arg;
    
    int new_socket = connection_dequeue(&queue);
   
    char resource_name[512];
    bzero(resource_name, sizeof(resource_name));
    if (read_http_request(new_socket, resource_name) != 0)
    {
        printf("read_http_request failed");
        close(new_socket);
        return NULL;
    }


    char path[512];
    bzero(path, sizeof(path));
    strcat(path, server_dir); // error check
    if (strcmp(resource_name, "/") == 0)
        strcat(path, "/index.html");
    else
        strcat(path, resource_name);

    if (write_http_response(new_socket, path) != 0)
    {
        fprintf(stderr, "write_http_request failed\n");
        close(new_socket);
        return NULL;
    }
    return NULL;
}

int main(int argc, char **argv)
{
    // First command is directory to serve, second command is port

    if (argc != 3)
    {
        printf("Usage: %s <directory> <port>\n", argv[0]);
        return 1;
    }
    char *serve_dir = argv[1];
    char *port = argv[2];

    struct sigaction sigact;
    sigact.sa_handler = handle_sigint;
    sigfillset(&sigact.sa_mask);
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
    if (ret_val != 0)
    {
        fprintf(stderr, "getaddrinfo failed: %s\n,", gai_strerror(ret_val));
        return 1;
    }
    int sock_fd = 0;
    if ((sock_fd = socket(server->ai_family, server->ai_socktype, server->ai_protocol)) == -1)
    {
        perror("socket");
        freeaddrinfo(server);
        return 1;
    }

    if (bind(sock_fd, server->ai_addr, server->ai_addrlen) == -1)
    {
        perror("bind");
        freeaddrinfo(server);
        close(sock_fd);
        return 1;
    }

    freeaddrinfo(server);

    if (listen(sock_fd, LISTEN_QUEUE_LEN) == -1)
    {
        perror("listen");
        close(sock_fd);
        return 1;
    }

    if (connection_queue_init(&queue) == -1)
    {
        fprintf(stderr, "Failed to initialize the queue\n");
        return 1;
    }

    pthread_t threads[N_THREADS];
    int result = 0;
    sigset_t mask, old_set;
    sigemptyset(&mask);
    sigfillset(&mask);
    sigprocmask(SIG_BLOCK, &mask, &old_set);
    for (int i = 0; i < N_THREADS; i++)
    {
        if ((result = pthread_create(threads + i, NULL, thread_func, serve_dir)) == -1)
        {
            fprintf(stderr, "pthread_create: %s\n", strerror(result));
            return 1;
        }
    }
    sigprocmask(SIG_SETMASK, &old_set, NULL);
    while (keep_going != 0)
    {
        int new_socket = accept(sock_fd, NULL, NULL);
        if (new_socket < 0)
        {
            if (errno != EINTR)
            {
                perror("accept connection");
                close(sock_fd);
                return 1;
            }
            else
                break;
        }
        connection_enqueue(&queue, new_socket);
    }

    if (close(sock_fd) == -1)
    {
        perror("close");
        return 1;
    }
   
    for (int i = 0; i < N_THREADS; i++){
        if (pthread_join(threads[i], NULL) != 0){
            fprintf(stderr, "Failed to join threads\n");
            return 1;
        }
    }
    return 0;
}