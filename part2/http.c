#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include "http.h"
#include <stdlib.h>

#define BUFSIZE 512

const char *get_mime_type(const char *file_extension) {
    if (strcmp(".txt", file_extension) == 0) {
        return "text/plain";
    } else if (strcmp(".html", file_extension) == 0) {
        return "text/html";
    } else if (strcmp(".jpg", file_extension) == 0) {
        return "image/jpeg";
    } else if (strcmp(".png", file_extension) == 0) {
        return "image/png";
    } else if (strcmp(".pdf", file_extension) == 0) {
        return "application/pdf";
    }

    return NULL;
}

int read_http_request(int fd, char *resource_name) {

    // copied from part 1
    char buf[BUFSIZE];
    bzero(buf, sizeof(buf));
    if(read(fd, buf, BUFSIZE) < 0){
        fprintf(stderr, "failed to read");
        return 1;
    }

    char *str = strtok(buf, " ");
    str = strtok(NULL, " ");//geting the file path
    if(str == NULL)return 1;
    strcpy(resource_name, str);
    // end copied section

    return 0;
}

int write_http_response(int fd, const char *resource_path) {
    
    // copied from part 1
    char response[BUFSIZE];
    bzero(response, sizeof(response));
    struct stat *buf;
    buf  = malloc(sizeof(struct stat));
    char *error_file = "server_files/error.html";
    int file;
    if(stat(resource_path, buf) == -1){
        errno = ENOENT;
        strcpy(response, "HTTP/1.1 404 NOT FOUND\r\n\n");
        file = open(error_file, O_RDONLY);

    }else{
        char *f_extension = strchr(resource_path, '.');
        file = open(resource_path, O_RDONLY);
        sprintf(response,"HTTP/1.0 200 OK\r\nContent-Type: %s\nContent-Length: %ld\r\n\r\n",get_mime_type(f_extension), buf->st_size);
    }
    free(buf);
    fprintf(stderr, "%s", response);
    if(write(fd, response, strlen(response)) < 0){
        fprintf(stderr, "failed to write reponse to client");
        return 1;
    }
    
    if(file < 0){
        fprintf(stderr, "failed to open file\n");
        return 1;
    }
    char content_buffer[4096];
    bzero(content_buffer, sizeof(content_buffer));
    int bytes_read;
    while((bytes_read = read(file, content_buffer, sizeof(content_buffer))) > 0){
        if(write(fd, content_buffer, bytes_read) < 0){
            fprintf(stderr, "failed to write content to client");
            close(file);
            return 1;
        }
    }
    
    
    close(file);
    // end copied from part1

    return 0;
}
