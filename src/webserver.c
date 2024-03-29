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
#include "status.h"
#include "util.h"


config_t config;

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
  if (http_req_str == NULL) {
    // return error
  }

  HttpRequest req = parse_http_request(http_req_str);
  HttpResponse res = build_http_response_struct(req);

  // run middlewares
  if (apply_middlewares(req, res) != 0) {
    // render error page
    render_html_error(res);
  }


  char *result = build_http_response_string(res);
  int res_size = strlen(result);

  write(socket_fd, result, res_size);
  free(http_req_str);
  free(result);
  free_request(req);
  free_response(res);


  // shut down process and close connection
  shutdown(socket_fd, SHUT_RDWR);
  close(socket_fd);
  exit(0);
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
  register_middleware(date_header_mw);
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
