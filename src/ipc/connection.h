#ifndef IPC_CONNECTION_H
#define IPC_CONNECTION_H

#include <stddef.h>

struct connection {
  int* sock;
  int* msgsock;
  struct sockaddr_un* server;
};

int connection_comparator(const struct connection* c1, const struct connection* c2);
void clearFreeConnection(struct connection* con) ;

#endif // IPC_CONNECTION_H
