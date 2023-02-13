/* Library for socket abstraction.
   Copyright (C) James S. Plank */

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int serve_socket(char *hn, int port) {
  struct sockaddr_in sn;
  int s;
  struct hostent *he;

  if (!(he = gethostbyname(hn))) {
    puts("can't gethostname");
    exit(1);
  }

  bzero((char *)&sn, sizeof(sn));
  sn.sin_family = AF_INET;
  sn.sin_port = htons((short)port);
  sn.sin_addr = *(struct in_addr *)(he->h_addr_list[0]);

  if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket()");
    exit(1);
  }
  if (bind(s, (struct sockaddr *)&sn, sizeof(sn)) == -1) {
    perror("bind()");
    exit(1);
  }

  return s;
}

int accept_connection(int s) {
  unsigned int l;
  struct sockaddr_in sn;
  int x;

  sn.sin_family = AF_INET;

  if (listen(s, 1) == -1) {
    perror("listen()");
    exit(1);
  }

  l = sizeof(sn);
  if ((x = accept(s, (struct sockaddr *)&sn, &l)) == -1) {
    perror("accept()");
    exit(1);
  }
  return x;
}
