#pragma once
#include "jrb.h"
#include "status.h"

typedef struct http_request {
  char *version;
  char *method;
  char *path;
  JRB headers;
  char *body;
} *HttpRequest;


typedef struct http_response {
  char *version;
  http_status status;
  JRB headers;
  char *body;
} *HttpResponse;

void render_html_error(HttpResponse res);

// core
HttpRequest parse_http_request(char *request_str);
HttpResponse build_http_response_struct(HttpRequest req);
char *build_http_response_string(HttpResponse res);

// helpers
void add_header_to_response(HttpResponse res, char *header, char *value);
char *get_header_str(HttpResponse res); // only in header for testing
char *set_response_body(HttpResponse res, char *body);
char *find_response_header(HttpResponse res, char *header);

void free_response(HttpResponse res);
void free_request(HttpRequest req);
void print_headers(HttpResponse res);
