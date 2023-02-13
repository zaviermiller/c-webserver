#include "status.h"

char *human_readable_status(http_status status) {
  switch (status) {
    case HTTP_OK:
      return "OK";
    case HTTP_NOT_FOUND:
      return "Not Found";
    case HTTP_SERVER_ERR:
      return "Internal Server Error";
    default:
      return "Status Not Handled";
  }
}
