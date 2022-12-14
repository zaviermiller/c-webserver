#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <time.h>
#include "socket.h"
#include "dllist.h"
#include "middleware.h"
#include "http.h"

char *canned_http_response =
"HTTP/1.1 200 OK\n"
"Date: Thu, 19 Feb 2009 12:27:04 GMT\n"
"Server: Apache/2.2.3\n"
"Last-Modified: Wed, 18 Jun 2003 16:05:58 GMT\n"
"ETag: \"56d-9989200-1132c580\"\n"
"Content-Type: text/html\n"
"Content-Length: 15\n"
"Accept-Ranges: bytes\n"
"Connection: close\n"
"\n"
"Hello, world";


int create_daemon(int socket_fd) {
  switch(fork()) {
    case -1: return -1;
    case 0: break;
    default: return 0; // parent should return to accept new connections
  }

  //if (setsid() == -1) {
  //  return -1;
  //}

  switch(fork()) {
    case -1: return -1;
    case 0: break;
    default: _exit(EXIT_SUCCESS);
  }

  HttpRequest req = read_request(socket_fd);
  HttpResponse res = build_response(req->version);
  res->status = HTTP_OK;

  // apply middlewares
  apply_middlewares(req, res);

  send_response(socket_fd, res);

  // shut down process and close connection
  shutdown(socket_fd, SHUT_RDWR);
  close(socket_fd);
  exit(0);
}

void put_file_contents_in_res_body(char* path, HttpResponse res) {
  FILE *f = fopen(path, "r");
  res->body = malloc(100000);
  fread(res->body, 1, 100000, f);
  fclose(f);
}

int render_file_mw(HttpRequest req, HttpResponse res) {
  // TODO: find file at relative path, open it and read its contents into a buffer. then put it in the body
  if (strcmp(req->method, "GET") != 0) return 0;

  struct stat path_stat;
  char *relative_path = malloc(strlen(req->path) + 10);
  strcpy(relative_path, ".");
  strcat(relative_path, req->path);
  if (access(relative_path, F_OK) != 0) {
    res->status = HTTP_NOT_FOUND;
    return 1;
  }

  stat(relative_path, &path_stat);

  if (S_ISDIR(path_stat.st_mode)) {
    // look for index html
    char *index_path = malloc(strlen(relative_path) + 10);
    strcpy(index_path, relative_path);

    // create index html path
    if (index_path[strlen(index_path) - 1] == '/') {
      strcat(index_path, "index.html");
    } else {
      strcat(index_path, "/index.html");
    }

    // stat to get info
    if (access(index_path, F_OK) != 0) {
      // render dir for now just return 404
      res->status = 404;
      return 1;
    }

    // render index
    put_file_contents_in_res_body(index_path, res);
    free(index_path);

  } else if (S_ISREG(path_stat.st_mode)) {
    // open the file, read its contents and put into body

    put_file_contents_in_res_body(relative_path, res);
  }

  free(relative_path);

  return 0;
}

int main(int argc, char **argv) {
  int port, socket, socket_fd;
  char hostname[15] = "0.0.0.0";

  if (argc < 2) {
    fprintf(stderr, "usage: %s port\n", argv[0]);
  }

  // TODO: error check
  port = atoi(argv[1]);

  middlewares = new_dllist();
  
  // register middlewares here
  register_middleware(render_file_mw);

  socket = serve_socket(hostname, port);
  printf("Listening at %s:%d\n", hostname, port);

  while (1) {
    socket_fd = accept_connection(socket);

    if (create_daemon(socket_fd) != 0) {
      perror("create_daemon");
      exit(1);
    }
    
  }

  return 0;
}

