#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include "socket.h"
#include "dllist.h"
#include "middleware.h"
#include "http.h"
#include "config.h"
#include "html.h"
#include "util.h"

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

  // handle http response
  handle_http(socket_fd);

  // shut down process and close connection
  shutdown(socket_fd, SHUT_RDWR);
  close(socket_fd);
  exit(0);
}

char *build_dir_page(char *path, char *relative_path) {
  // pretty simple, just need to create a header with the url, and then links to each page in it
  DIR *d;
  struct dirent *de;
  char *a_tag;
  char a_href[strlen(path) + 1000];
  d = opendir(relative_path);

  char header_str[10 + strlen(path)];
  sprintf(header_str, "Index of %s", path);
  char *res = malloc(10000);
  int offset = 0;
  char *h1 = build_h1(header_str);
  sprintf(res, "%s\n", h1);
  free(h1);

  if (d) {
    while ((de = readdir(d)) != NULL) {
      if (strncmp(de->d_name, ".", 1) != 0 && strncmp(de->d_name, "..", 2) != 0) {
        strcpy(a_href, path);
        if (path[strlen(path) - 1] != '/') {
          strcat(a_href, "/");
        }

        strcat(a_href, de->d_name);
        a_tag = build_a(de->d_name, a_href);
        sprintf(res + strlen(res), "%s<br />\n", a_tag);
        free(a_tag);
      }
    }
  }
  
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
      res->body = build_dir_page(req->path, relative_path);
    } else {
      // render index
      res->body = get_file_contents(index_path);
    }

  } else if (S_ISREG(path_stat.st_mode)) {
    // open the file, read its contents and put into body

    res->body = get_file_contents(relative_path);
  }

  if (res->body == NULL) {
    res->status = HTTP_SERVER_ERR;
    return 1;
  }

  return 0;
}

int wrap_pre_mw(HttpRequest req, HttpResponse res) {
  char *ext = get_extension(req->path);

  if (is_dir(req->path)) return 0;
  if (ext != NULL && strncmp(ext, "html", 4) == 0) return 0;

  char *tmp = malloc(strlen(res->body) + 100);
  sprintf(tmp, "<pre style=\"word-wrap: break-word; white-space: pre-wrap;\">\n%s\n</pre>", res->body);
  free(res->body);
  res->body = tmp;

  return 0;
}

int escape_html_mw(HttpRequest req, HttpResponse res) {
  char *ext = get_extension(req->path);

  if (ext == NULL) return 0;
  if (strncmp(ext, "html", 4) == 0) return 0;
  
  char *tmp = escape_html(res->body);
  free(res->body);
  res->body = tmp;

  return 0;
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
    if (!dir_exists(argv[2])) {
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
  // register_middleware(escape_html_mw);
  register_middleware(wrap_pre_mw);

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

