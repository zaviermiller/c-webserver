#include "html.h"
#include <stdlib.h>
#include <stdio.h>

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
