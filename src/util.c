#include "util.h"
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int dir_exists(char *root_dir) {
  struct stat s;

  if (access(root_dir, F_OK) != 0) return 0;

  if (stat(root_dir, &s) != 0) return 1;

  if (!S_ISDIR(s.st_mode)) return 0;

  return 1;
}

// needs to be able to handle bigger files
char *get_file_contents(char* path) {
  FILE *f = fopen(path, "r");
  char *res = malloc(1000);

  fread(res, 1000, 1, f);

  return res;
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
