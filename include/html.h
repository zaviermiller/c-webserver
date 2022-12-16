#pragma once

#include "http.h"

void render_html_error(HttpResponse res);
char *build_h1(char *text);
char *build_a(char *text, char *href);
char *escape_html(char *text);
