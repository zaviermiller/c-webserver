#pragma once

#include "http.h"

int date_header_mw(HttpRequest req, HttpResponse res);
int render_file_mw(HttpRequest req, HttpResponse res);
int wrap_pre_mw(HttpRequest req, HttpResponse res);
int escape_html_mw(HttpRequest req, HttpResponse res);
