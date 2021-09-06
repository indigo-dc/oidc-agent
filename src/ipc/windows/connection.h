#ifndef IPC_CONNECTION_H
#define IPC_CONNECTION_H

#include <stddef.h>
#include <winsock2.h>

struct connection {
  int msys_secret[4];
  struct sockaddr_in* msys_server;
  SOCKET* sock;
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
