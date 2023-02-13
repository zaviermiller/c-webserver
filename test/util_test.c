#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
  int fd = open("test/test.txt", O_RDONLY);
  char *str = readall(fd);
  close(fd);
  free(str);

  fd = open("test/test.txt", O_RDONLY);
  str = readall(fd);
  close(fd);
  printf("%s", str);
  free(str);
}
