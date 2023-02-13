#include "http.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "jrb.h"
#include "string.h"

#define SERVER_NAME "ZServer"
#define DEFAULT_HEADER_STRLEN 1024

HttpResponse build_response(char *http_version) {
  HttpResponse res = malloc(sizeof(struct http_response));
  res->version = strdup(http_version);
  res->headers = make_jrb();

  add_header_to_response(res, "Content-Type", "text/html; charset=UTF-8");
  add_header_to_response(res, "Server", SERVER_NAME);
  add_header_to_response(res, "Content-Length", "0");

  res->body = NULL;

  return res;
}

HttpResponse build_http_response_struct(HttpRequest req) {
  HttpResponse res = malloc(sizeof(struct http_response));
  res->version = strdup(req->version);
  res->headers = make_jrb();
  res->status = HTTP_OK;

  // add required headers
  add_header_to_response(res, "Content-Type", "text/html; charset=UTF-8");
  add_header_to_response(res, "Server", SERVER_NAME);
  // add_header_to_response(res, "Content-Length", "0");

  res->body = NULL;

  return res;
}

char *set_response_body(HttpResponse res, char *body) {
  res->body = strdup(body);

  return res->body;
}

void free_response(HttpResponse res) {
  JRB ptr;
  free(res->version);
  free(res->body);

  jrb_traverse(ptr, res->headers) {
    free(ptr->key.v);
    free(ptr->val.v);
  }

  jrb_free_tree(res->headers);
  free(res);
}

HttpRequest parse_http_request(char *req_str) {
  HttpRequest req = malloc(sizeof(struct http_request));

  // loop through the string and find method, path, and version length
  int offset = 0;
  req->version = malloc(10);
  if (req->version == NULL) {
    return NULL;
  }

  for (int i = offset; i < strlen(req_str); i++) {
    if (req_str[i] == ' ') {
      int method_len = i - offset;
      req->method = calloc(method_len + 1, 1);
      if (req->method == NULL) return NULL;
      strncpy(req->method, req_str + offset, method_len);
      offset = i + 1;
      goto found_method;
    }
  }

  return NULL;

found_method:
  for (int i = offset; i < strlen(req_str); i++) {
    if (req_str[i] == ' ') {
      int len = i - offset;
      req->path = calloc(len + 1, 1);
      if (req->path == NULL) return NULL;
      strncpy(req->path, req_str + offset, len);
      offset = i + 1;
      goto found_path;
    }
  }

  return NULL;

found_path:
  sscanf(req_str + offset, "HTTP/%s", req->version);

// todo: handle body and headers and add error codes or something

  return req;
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

char *get_header_str(HttpResponse res) {
  char *header_str = calloc(DEFAULT_HEADER_STRLEN, 1), *header_key, *header_val;
  int header_len = 0;
  int alloced_size = DEFAULT_HEADER_STRLEN;
  JRB ptr;
  jrb_traverse(ptr, res->headers) {
    /* check for headers we want to control ourselves */
    header_key = ptr->key.s;
    header_val = ptr->val.s;

    header_len = strlen(header_str);

    if (header_len + 1 >= alloced_size) {
      header_str = realloc(header_str, alloced_size + DEFAULT_HEADER_STRLEN);
      alloced_size += DEFAULT_HEADER_STRLEN;
    }

    sprintf(header_str + header_len, "%s: %s\n", header_key, header_val);
  }

  return header_str;
}

char *stringify_response(HttpResponse res) {
  // char *date_str = get_date_str();

  int body_len = strlen(res->body);
  char body_len_str[50];
  sprintf(body_len_str, "%d", body_len);
  add_header_to_response(res, "Content-Length", body_len_str);
  char *header_str = get_header_str(res);

  char *res_str = calloc(strlen(header_str) + body_len + 128, 1);
  sprintf(res_str, "HTTP/%s %d %s\n", res->version, res->status,
          human_readable_status(res->status));
  strcat(res_str, header_str);
  sprintf(res_str + strlen(res_str), "\n%s", res->body);

  //free(date_str);
  free(header_str);

  return res_str;
}

char *build_http_response_string(HttpResponse res) {

  int body_len = 0;
  if (res->body != NULL && find_response_header(res, "Content-Length") == NULL) {
    body_len = strlen(res->body);
    char body_len_str[50];
    sprintf(body_len_str, "%d", body_len);
    add_header_to_response(res, "Content-Length", body_len_str);
  }
  char *header_str = get_header_str(res);

  char *res_str = calloc(strlen(header_str) + body_len + 128, 1);
  sprintf(res_str, "HTTP/%s %d %s\n", res->version, res->status,
          human_readable_status(res->status));
  strcat(res_str, header_str);
  sprintf(res_str + strlen(res_str), "\n%s", res->body);

  // free(date_str);
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

  write(socket_fd, res_string, strlen(res_string) + 1);

  free(res_string);
}

void add_header_to_response(HttpResponse res, char *header, char *value) {
  JRB ptr = jrb_find_str(res->headers, header);

  if (ptr == NULL) {
    // if header doesn't already exist
    jrb_insert_str(res->headers, strdup(header), new_jval_s(strdup(value)));
  } else {
    // if header does exist
    free(ptr->val.s);
    ptr->val = new_jval_s(strdup(value));
  }
}

char *find_response_header(HttpResponse res, char *key) {
  JRB ptr = jrb_find_str(res->headers, key);
  if (ptr == NULL) return NULL;

  return ptr->val.s;

}


void print_headers(HttpResponse res) {
  JRB ptr;
  jrb_traverse(ptr, res->headers) {
    printf("%s: %s\n", ptr->key.s, ptr->val.s);
  }
}
