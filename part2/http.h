#ifndef HTTP_H
#define HTTP_H

int read_http_request(int fd, char *resource_name);

int write_http_response(int fd, const char *resource_path);

#endif // HTTP_H
