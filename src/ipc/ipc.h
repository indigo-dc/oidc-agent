#ifndef IPC_H
#define IPC_H

#include "../oidc_error.h"
#include "connection.h"
#include "ipc_values.h"

#include <stdarg.h>

oidc_error_t ipc_init(struct connection* con, const char* env_var_name, int isServer) ;
oidc_error_t ipc_initWithPath(struct connection* con) ;

int ipc_bindAndListen(struct connection* con) ;
int ipc_connect(struct connection con) ;

char* ipc_read(int _sock);

oidc_error_t ipc_write(int _sock, char* msg, ...);
oidc_error_t ipc_vwrite(int _sock, char* msg, va_list args);
oidc_error_t ipc_writeOidcErrno(int sock) ;

oidc_error_t ipc_close(struct connection* con);
oidc_error_t ipc_closeAndUnlink(struct connection* con);

static char* server_socket_path = NULL;

#endif // IPC_H
