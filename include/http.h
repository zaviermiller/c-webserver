#pragma once
#include "jrb.h"

typedef struct http_request {
  char *version;
  char *method;
  char *path;
  JRB headers;
  char *body;
} *HttpRequest;

typedef enum http_response_status { HTTP_OK = 200, HTTP_FOUND = 302, HTTP_BAD_REQUEST = 400, HTTP_UNAUTHORIZED = 401,
  HTTP_NOT_FOUND = 404, HTTP_SERVER_ERR = 500 } http_status;

typedef struct http_response {
  char *version;
  http_status status;
  JRB headers;
  char *body;
} *HttpResponse;

int handle_http(int fd);

HttpResponse build_response(char *http_version);
void send_response(int socket_fd, HttpResponse res);
HttpRequest read_request(int socket_fd);
void add_header_to_response(HttpResponse res, char *header, char *value);
void render_html_error(HttpResponse res);

void free_response(HttpResponse res);
void free_request(HttpRequest req);
