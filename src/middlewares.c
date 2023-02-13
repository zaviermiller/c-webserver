#include "middlewares.h"

#include <stdlib.h>
#include <time.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>

#include "http.h"
#include "html.h"
#include "status.h"
#include "util.h"
#include "config.h"

extern config_t config;

char *get_date_str() {
  char *date = malloc(100);
  time_t raw_time;
  struct tm *info;

  time(&raw_time);

  info = gmtime(&raw_time);

  strftime(date, 100, "%c", info);

  return date;
}

int date_header_mw(HttpRequest req, HttpResponse res) {
  char *date_str = get_date_str();
  if (date_str == NULL) {
    res->status = HTTP_SERVER_ERR;
    return 1;
  }

  add_header_to_response(res, "Date", date_str);

  free(date_str);

  return 0;
}

char *build_dir_page(char *path, char *relative_path) {
  // pretty simple, just need to create a header with the url, and then links to
  // each page in it
  DIR *d;
  struct dirent *de;
  char *a_tag, *res, *h1;
  char a_href[strlen(path) + 1000];
  if ((d = opendir(relative_path)) == NULL) {
    return NULL;
  }

  char header_str[10 + strlen(path)];
  if ((sprintf(header_str, "Index of %s", path)) < 0) {
    return NULL;
  }
  if ((res = malloc(10000)) == NULL) {
    return NULL;
  }
  int offset = 0;
  if ((h1 = build_h1(header_str)) == NULL) {
    return NULL;
  }
  if ((sprintf(res, "%s\n", h1)) < 0) {
    return NULL;
  }
  free(h1);

  if (d) {
    while ((de = readdir(d)) != NULL) {
      if (strncmp(de->d_name, ".", 1) != 0 &&
          strncmp(de->d_name, "..", 2) != 0) {
        strcpy(a_href, path);
        if (path[strlen(path) - 1] != '/') {
          strcat(a_href, "/");
        }

        strcat(a_href, de->d_name);
        if ((a_tag = build_a(de->d_name, a_href)) == NULL) return NULL;
        if ((sprintf(res + strlen(res), "%s<br />\n", a_tag)) < 0) {
          free(a_tag);
          return NULL;
        }
        free(a_tag);
      }
    }
  }

  closedir(d);

  return res;
}

int render_file_mw(HttpRequest req, HttpResponse res) {
  if (strcmp(req->method, "GET") != 0) return 0;

  char *body;

  struct stat path_stat;
  char relative_path[strlen(req->path) + strlen(config.root_dir)];
  strcpy(relative_path, config.root_dir);
  strcat(relative_path, req->path);
  if (access(relative_path, F_OK) != 0) {
    res->status = HTTP_NOT_FOUND;
    return 1;
  }

  if (stat(relative_path, &path_stat) != 0) {
    res->status = HTTP_SERVER_ERR;
    return 1;
  }

  if (S_ISDIR(path_stat.st_mode)) {
    // look for index html
    char index_path[strlen(relative_path) + 15];
    strcpy(index_path, relative_path);

    // create index html path
    if (index_path[strlen(index_path) - 1] == '/') {
      strcat(index_path, "index.html");
    } else {
      strcat(index_path, "/index.html");
    }

    // stat to get info
    if (access(index_path, F_OK) != 0) {
      // render dir for now just return 404
      body = build_dir_page(req->path, relative_path);
    } else {
      // render index
      body = get_file_contents(index_path);
    }

  } else if (S_ISREG(path_stat.st_mode)) {
    // open the file, read its contents and put into body

    body = get_file_contents(relative_path);
  }

  if (body == NULL) {
    res->status = HTTP_SERVER_ERR;
    return 1;
  }

  set_response_body(res, body);
  free(body);

  return 0;
}

int wrap_pre_mw(HttpRequest req, HttpResponse res) {
  char *ext = get_extension(req->path);

  if (is_dir(req->path)) return 0;
  if (ext != NULL && strncmp(ext, "html", 4) == 0) return 0;

  char *tmp = malloc(strlen(res->body) + 100);
  sprintf(tmp,
          "<pre style=\"word-wrap: break-word; white-space: "
          "pre-wrap;\">\n%s\n</pre>",
          res->body);
  free(res->body);
  res->body = tmp;

  return 0;
}

int escape_html_mw(HttpRequest req, HttpResponse res) {
  char *ext = get_extension(req->path);

  if (ext == NULL) return 0;
  if (strncmp(ext, "html", 4) == 0) return 0;

  char *tmp = escape_html(res->body);
  free(res->body);
  res->body = tmp;

  return 0;
}
