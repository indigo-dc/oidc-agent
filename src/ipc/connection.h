#ifndef IPC_CONNECTION_H
#define IPC_CONNECTION_H

#include <stddef.h>

struct connection {
  int*                sock;
  int*                msgsock;
  struct sockaddr_un* server;
  struct sockaddr_in* tcp_server;
};

int  connection_comparator(const struct connection* c1,
                           const struct connection* c2);
void _secFreeConnection(struct connection* con);

#ifndef secFreeConnection
#define secFreeConnection(ptr) \
  do {                         \
    _secFreeConnection((ptr)); \
    (ptr) = NULL;              \
  } while (0)
#endif  // secFreeConnection

#endif  // IPC_CONNECTION_H
