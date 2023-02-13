#pragma once

typedef enum http_response_status { HTTP_OK = 200, HTTP_FOUND = 302, HTTP_BAD_REQUEST = 400, HTTP_UNAUTHORIZED = 401,
  HTTP_NOT_FOUND = 404, HTTP_SERVER_ERR = 500 } http_status;

char *human_readable_status(http_status status);
