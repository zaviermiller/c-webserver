#include "html.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char *display_status(int status) {
  switch(status) {
    case HTTP_OK: return "Ok";
    case HTTP_NOT_FOUND: return "Not Found";
    case HTTP_SERVER_ERR: return "Internal Server Error";
    default: return "Unknown Status";
  }
}

void render_html_error(HttpResponse res) {
  if (res->body == NULL) {
    res->body = malloc(1000);
  }

  sprintf(res->body, "<h1 style=\"text-align: center\">%d %s</h1>\n", res->status, display_status(res->status));
}

char *build_h1(char *text) {
  char *res = malloc(strlen(text) + 10);
  sprintf(res, "<h1>%s</h1>", text);
  return res;
}

char *build_a(char *text, char *href) {
  char *res = malloc(strlen(text) + strlen(href) + 50);
  sprintf(res, "<a href=\"%s\">%s</a>", href, text);
  return res;
}
