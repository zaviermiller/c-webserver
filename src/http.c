#include "http.h"
#include "jrb.h"
#include "string.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

#define SERVER_NAME "ZServer"

HttpResponse build_response(char *http_version) {
  HttpResponse res = malloc(sizeof(struct http_response));
  res->version = strdup(http_version);
  res->headers = make_jrb();

  add_header_to_response(res, "Content-Type", "text/html; charset=UTF-8");
  add_header_to_response(res, "Server", SERVER_NAME);
  add_header_to_response(res, "Content-Length", "5");

  res->body = NULL;

  return res;
}

void free_response(HttpResponse res) {
  free(res->version);
  free(res);
}

HttpRequest parse_request(char *req_str) {
  HttpRequest req = malloc(sizeof(struct http_request));
  req->method = malloc(100);
  req->path = malloc(1000);
  req->version = malloc(10);

  // TODO: handle body and headers

  sscanf(req_str, "%s %s HTTP/%s", req->method, req->path, req->version);

  return req;
}

void free_request(HttpRequest req) {
  free(req->method);
  free(req->path);
  free(req->version);

  free(req);
}

char *human_readable_status(int status) {
  switch(status) {
    case HTTP_OK: return "OK";
    case HTTP_NOT_FOUND: return "NOT FOUND";
    default: return "INVALID STATUS";
  }
}

char *get_date_str() {
  char *date = malloc(1000);
  time_t raw_time;
  struct tm *info;

  time(&raw_time);

  info = gmtime(&raw_time);

  strftime(date, 1000, "%c", info);

  return date;
}

char *get_header_str(HttpResponse res) {
  char *header_str = calloc(1000, 0), *header_key, *header_val;
  char single_header[1000];
  int header_len = 0;
  int alloced_size = 1000;
  JRB ptr;
  jrb_traverse(ptr, res->headers) {
    /* check for headers we want to control ourselves */
    header_key = ptr->key.s;
    header_val = ptr->val.s;

    header_len = strlen(header_str);

    if (header_len + 1 >= alloced_size) {
      header_str = realloc(header_str, 1000);
      alloced_size += 1000;
    }

    sprintf(header_str + header_len, "%s: %s\n", header_key, header_val);
  }

  return header_str;
}

char *stringify_response(HttpResponse res) {
  char *res_str = malloc(1000);
  char *date_str = get_date_str();

  int body_len = strlen(res->body);
  char body_len_str[50];
  sprintf(body_len_str, "%d", body_len);
  add_header_to_response(res, "Content-Length", body_len_str);
  char *header_str = get_header_str(res);


  sprintf(res_str, "HTTP/%s %d %s\nDate: %s\n", res->version, res->status, human_readable_status(res->status), date_str);
  strcat(res_str, header_str);
  sprintf(res_str + strlen(res_str), "\n%s", res->body);

  free(date_str);
  free(header_str);

  return res_str;
}

HttpRequest read_request(int socket_fd) {
  char req_str[100000];

  read(socket_fd, req_str, 100000);

  return parse_request(req_str);
}

void send_response(int socket_fd, HttpResponse res) {

  char *res_string = stringify_response(res);

  printf("response:\n%s\n\n", res_string);

  write(socket_fd, res_string, strlen(res_string) + 1);

  free(res_string);
}

void add_header_to_response(HttpResponse res, char *header, char *value) {
  JRB ptr = jrb_find_str(res->headers, header);

  if (ptr == NULL) {
    jrb_insert_str(res->headers, strdup(header), new_jval_s(strdup(value)));
  } else {
    free(ptr->val.s);
    ptr->val = new_jval_s(strdup(value));
  }
}

