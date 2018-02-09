#ifndef IPC_H
#define IPC_H

#include "oidc_error.h"
#include "ipc_values.h"

#include <stdarg.h>

struct connection {
  int* sock;
  int* msgsock;
  struct sockaddr_un* server;
};

char* init_socket_path(const char* env_var_name) ;
oidc_error_t ipc_init(struct connection* con, const char* env_var_name, int isServer) ;
oidc_error_t ipc_initWithPath(struct connection* con) ;
int ipc_bindAndListen(struct connection* con) ;
struct connection* ipc_async(struct connection listencon, struct connection** clientcons_addr, size_t* size) ;
int ipc_connect(struct connection con) ;
char* ipc_read(int _sock);
oidc_error_t ipc_write(int _sock, char* msg, ...);
oidc_error_t ipc_vwrite(int _sock, char* msg, va_list args);
oidc_error_t ipc_writeOidcErrno(int sock) ;
oidc_error_t ipc_close(struct connection* con);
oidc_error_t ipc_closeAndUnlink(struct connection* con);

struct connection* addConnection(struct connection* cons, size_t* size, struct connection client) ;
struct connection* findConnection(struct connection* cons, size_t size, struct connection key) ;
struct connection* removeConnection(struct connection* cons, size_t* size, struct connection* key) ;

static char* server_socket_path = NULL;

#endif // IPC_H
