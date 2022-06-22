#ifndef IPC_CONNECTION_H
#define IPC_CONNECTION_H

#include <stddef.h>

#include "defines/msys.h"
#ifdef MINGW
#include <winsock2.h>
#else
#include <netinet/in.h>
#include <sys/un.h>

#include "socket.h"
#endif

struct connection {
  SOCKET* sock;
#ifndef MINGW
  SOCKET*             msgsock;
  struct sockaddr_un* server;
#endif
  struct sockaddr_in* tcp_server;
#ifdef MINGW
  int msys_secret[4];
#endif
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
