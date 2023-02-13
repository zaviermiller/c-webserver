#include "middleware.h"

Dllist middlewares;

void register_middleware(HttpMiddleware mw) {
  dll_append(middlewares, new_jval_v(mw));
}

int apply_middlewares(HttpRequest req, HttpResponse res) {
  Dllist ptr;
  HttpMiddleware hm;
  dll_traverse(ptr, middlewares) {
    hm = ptr->val.v;
    if (hm(req, res) != 0) {
      return 1;
    }
  }

  return 0;
}
