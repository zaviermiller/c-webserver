#include "html.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *display_status(int status) {
  switch (status) {
    case HTTP_OK:
      return "Ok";
    case HTTP_NOT_FOUND:
      return "Not Found";
    case HTTP_SERVER_ERR:
      return "Internal Server Error";
    default:
      return "Unknown Status";
  }
}

void render_html_error(HttpResponse res) {
  if (res->body == NULL) {
    res->body = malloc(1000);
  }

  sprintf(res->body, "<h1 style=\"text-align: center\">%d %s</h1>\n",
          res->status, display_status(res->status));
}

char *build_h1(char *text) {
  char *res;
  if ((res = malloc(strlen(text) + 10)) == NULL) {
    return NULL;
  }
  if ((sprintf(res, "<h1>%s</h1>", text)) < 0) return NULL;
  return res;
}

char *build_a(char *text, char *href) {
  char *res;
  if ((res = malloc(strlen(text) + strlen(href) + 50)) == NULL) return NULL;
  if ((sprintf(res, "<a href=\"%s\">%s</a>", href, text)) < 0) return NULL;
  return res;
}

char *escape_html(char *text) {
  char *res = malloc(strlen(text) + 1000);
  char *working = malloc(strlen(text) + 1000);
  strcpy(res, text);
  char *start, *end;
  for (int i = 0; i < strlen(res); i++) {
    switch (res[i]) {
      case '<':
        start = res + i - 1;
        end = res + i + 1;
        sprintf(working, "%s&lt;%s", start, end);
        res = working;
        printf("%s\n", res);
        return res;
      default:
        continue;
    }
  }

  return res;
}
