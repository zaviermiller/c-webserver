#include "http.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

void test_parse_http_request(char *reqstr) {

  HttpRequest req = parse_http_request(reqstr);

  assert(req != NULL);
  assert(strcmp(req->method, "GET") == 0);
  assert(strcmp(req->path, "/test/path") == 0);
  assert(strcmp(req->version, "1.1") == 0);

  free_request(req);
}

void test_build_http_response_struct(char *reqstr) {
  HttpRequest req = parse_http_request(reqstr);
  assert(req != NULL);

  HttpResponse res = build_http_response_struct(req);
  assert(res != NULL);
  
  assert(strcmp(res->version, "1.1") == 0);

  free_request(req);
  free_response(res);
}

void test_headers(char *reqstr) {
  HttpRequest req = parse_http_request(reqstr);
  assert(req != NULL);

  HttpResponse res = build_http_response_struct(req);
  assert(res != NULL);

  // add some headers to the response
  add_header_to_response(res, "Cool", "Header");
  add_header_to_response(res, "Hello", "World");
  add_header_to_response(res, "Man", "Stop");
  add_header_to_response(res, "Master", "Programmer");
  add_header_to_response(res, "New", "Guy");

  char *header_str = get_header_str(res);

  const char *correct_header_str = "Content-Type: text/html; charset=UTF-8\n"
                                   "Cool: Header\n"
                                   "Hello: World\n"
                                   "Man: Stop\n"
                                   "Master: Programmer\n"
                                   "New: Guy\n"
                                   "Server: ZServer\n";


  assert(strcmp(header_str, correct_header_str) == 0);
  
  free_request(req);
  free_response(res);
}

void test_build_http_response_string(char *reqstr) {
  HttpRequest req = parse_http_request(reqstr);
  assert(req != NULL);

  HttpResponse res = build_http_response_struct(req);
  assert(res != NULL);

  // add some headers to the response
  add_header_to_response(res, "Cool", "Header");
  add_header_to_response(res, "Hello", "World");
  add_header_to_response(res, "Man", "Stop");
  add_header_to_response(res, "Master", "Programmer");
  add_header_to_response(res, "New", "Guy");

  set_response_body(res, "Hello, world!");
  res->status = 200;

  char *response_str = build_http_response_string(res);

  printf("%s\n", response_str);

  free(response_str);

  free_request(req);
  free_response(res);
}

int main() {
  // assume readall works as expected
  char *reqstr = readall(0);

  printf("testing parse_http_request...");
  test_parse_http_request(reqstr);
  printf("pass\n");

  printf("testing build_http_response_struct...");
  test_build_http_response_struct(reqstr);
  printf("pass\n");

  printf("testing add_header_to_response and get_header_str...");
  test_headers(reqstr);
  printf("pass\n");

  // printf("testing build_http_response_struct...");
  test_build_http_response_string(reqstr);
  // printf("pass\n");

  free(reqstr);
}
