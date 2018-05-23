#ifndef IPC_CONNECTION_H
#define IPC_CONNECTION_H

#include <stddef.h>

struct connection {
  int* sock;
  int* msgsock;
  struct sockaddr_un* server;
};

struct connection* addConnection(struct connection* cons, size_t* size, struct connection client) ;
struct connection* findConnection(struct connection* cons, size_t size, struct connection key) ;
struct connection* removeConnection(struct connection* cons, size_t* size, struct connection* key) ;

#endif // IPC_CONNECTION_H
