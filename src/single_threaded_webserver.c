#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "dllist.h"
#include "html.h"
#include "http.h"
#include "middleware.h"
#include "middlewares.h"
#include "socket.h"
#include "util.h"


config_t config;

char *handle_request(char *http_req_str) {
  HttpRequest req = parse_http_request(http_req_str);
  HttpResponse res = build_http_response_struct(req);

  // run middlewares
  if (apply_middlewares(req, res) != 0) {
    // render error page
  }

  char *result = build_http_response_string(res);

  free_response(res);
  free_request(req);

  return result;
}

int create_daemon(int socket_fd) {
  switch (fork()) {
    case -1:
      return -1;
    case 0:
      break;
    default:
      return 0;  // parent should return to accept new connections
  }

  // if (setsid() == -1) {
  //   return -1;
  // }

  switch (fork()) {
    case -1:
      return -1;
    case 0:
      break;
    default:
      _exit(EXIT_SUCCESS);
  }

  // read in request from fd
  // parse, run middlewares, send
  char *http_req_str = readall(socket_fd);
  if (http_req_str == 0) {
    // return error
  }

  char *result = handle_request(http_req_str);
  int res_size = strlen(result);

  write(socket_fd, result, res_size);

  free(result);
  free(http_req_str);


  // shut down process and close connection
  shutdown(socket_fd, SHUT_RDWR);
  close(socket_fd);
  exit(0);
}

int main(int argc, char **argv) {
  int port, socket;
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
  register_middleware(date_header_mw);
  register_middleware(render_file_mw);
  // register_middleware(escape_html_mw);
  register_middleware(wrap_pre_mw);

  socket = serve_socket(hostname, port);
  // setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
  // setsockopt(socket, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));

  printf("Listening at %s:%d\n", hostname, port);

  while (1) {
    int socket_fd = accept_connection(socket);

    // read in request from fd
    // parse, run middlewares, send
    char *http_req_str = readall(socket_fd);
    if (http_req_str == 0) {
      // return error
    }

    char *result = handle_request(http_req_str);
    if (result == NULL) {
      return 1;
    }
    int res_size = strlen(result);

    write(socket_fd, result, res_size + 1);

    free(http_req_str);
    free(result);


    // shut down process and close connection
    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);
  }

  free_dllist(middlewares);

  return 0;
}
