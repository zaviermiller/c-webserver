#include <asm-generic/socket.h>
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
#include "config.h"

config_t config;

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

  free_response(res);
  free_request(req);

  // shut down process and close connection
  shutdown(socket_fd, SHUT_RDWR);
  close(socket_fd);
  exit(0);
}

// needs to be able to handle bigger files
char *get_file_contents(char* path) {
  FILE *f = fopen(path, "r");
  char *res = NULL;
  int res_len = 0;
  while (!feof(f)) {
    char *tmp = realloc(res, 1000);
    if (!tmp) {
      free(res);
      return NULL;
    } else {
      res = tmp;
    }
    fread(res + res_len, 1000, 1, f);
    res_len += 1000;
  }
  fclose(f);

  return res;
}

int render_file_mw(HttpRequest req, HttpResponse res) {
  // TODO: find file at relative path, open it and read its contents into a buffer. then put it in the body
  if (strcmp(req->method, "GET") != 0) return 0;

  struct stat path_stat;
  char relative_path[strlen(req->path) + strlen(config.root_dir)];
  strcpy(relative_path, config.root_dir);
  strcat(relative_path, req->path);
  if (access(relative_path, F_OK) != 0) {
    res->status = HTTP_NOT_FOUND;
    return 1;
  }

  if (stat(relative_path, &path_stat) != 0) {
    res->status = HTTP_SERVER_ERR;
    return 1;
  }

  if (S_ISDIR(path_stat.st_mode)) {
    // look for index html
    char index_path[strlen(relative_path) + 15];
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
    res->body = get_file_contents(index_path);

  } else if (S_ISREG(path_stat.st_mode)) {
    // open the file, read its contents and put into body

    res->body = get_file_contents(relative_path);
  }

  if (res->body == NULL) {
    res->status = 500;
    return 1;
  }

  return 0;
}

int validate_root_dir(char *root_dir) {
  struct stat s;

  if (access(root_dir, F_OK) != 0) return 0;

  if (stat(root_dir, &s) != 0) return 1;

  if (!S_ISDIR(s.st_mode)) return 0;

  return 1;
}

int main(int argc, char **argv) {
  int port, socket, socket_fd;
  char hostname[15] = "0.0.0.0";
  int reuse = 1;

  if (argc < 2) {
    fprintf(stderr, "usage: %s port [root dir]\n", argv[0]);
    return 1;
  }

  if (argc == 3) {
    if (!validate_root_dir(argv[2])) {
      fprintf(stderr, "%s: invalid root dir\n", argv[0]);
      return 1;
    }
    config.root_dir = argv[2];
  } else {
    config.root_dir = ".";
  }

  // TODO: error check
  port = atoi(argv[1]);

  middlewares = new_dllist();
  
  // register middlewares here
  register_middleware(render_file_mw);

  socket = serve_socket(hostname, port);
  setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
  setsockopt(socket, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));

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

