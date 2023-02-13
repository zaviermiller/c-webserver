#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFSIZE 4096

int dir_exists(char *root_dir) {
  struct stat s;

  if (access(root_dir, F_OK) != 0) return 0;

  if (stat(root_dir, &s) != 0) return 1;

  if (!S_ISDIR(s.st_mode)) return 0;

  return 1;
}

// needs to be able to handle bigger files
char *get_file_contents(char *path) {
  FILE *f = fopen(path, "r");
  char *buf = calloc(BUFSIZE, 1);

  int alloced = 0;
  while(fread(buf + alloced, BUFSIZE, 1, f) == 1) {
    alloced += BUFSIZE;
    char *realloced = realloc(buf, alloced + BUFSIZE);
    if (realloced == NULL) {
      return NULL;
    }
    buf = realloced;
    memset(buf + alloced, 0, BUFSIZE);
  }

  fclose(f);

  return buf;
}

char *get_extension(char *path) {
  char *ext = strrchr(path, '.');
  if (ext) return ext + 1;
  return NULL;
}

int is_dir(char *path) {
  char relative_path[strlen(path) + 5];
  struct stat s;
  strcpy(relative_path, ".");
  strcat(relative_path, path);

  if (stat(relative_path, &s) != 0) return 0;

  if (S_ISDIR(s.st_mode)) return 1;

  return 0;
}

// must be freed
char *readall(int fd) {
  char *buf = calloc(BUFSIZE, 1);
  if (buf <= 0) return NULL;

  int read_bytes = 0;
  int total = 0;
  while (read_bytes = read(fd, buf + total, BUFSIZE), read_bytes == BUFSIZE) {
    total += read_bytes;
    char *realloced = realloc(buf, BUFSIZE + total); // add extra BUFSIZE bytes to allocation
    if (realloced <= 0) {
      free(buf);
      return NULL;
    }

    buf = realloced;
    memset(buf + total, 0, BUFSIZE);
  }

  return buf;

}

char *to_lower(char *str) {
  for (int i = 0; i < strlen(str); i++) {
    char c = str[i];
    if (c <= 'Z' && c >= 'A') {
      str[i] = c + ('a' - 'A');
    }
  }

  return str;

}
