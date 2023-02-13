#pragma once
#include "dllist.h"
#include "http.h"

extern Dllist middlewares;

// middleware func should return 0 to continue, anything else to stop short
typedef int (*HttpMiddleware)(HttpRequest req, HttpResponse res);

void register_middleware(HttpMiddleware mw);
int apply_middlewares(HttpRequest req, HttpResponse res);

