#ifndef IPC_SERVER_H
#define IPC_SERVER_H

#include "connection.h"
#include "list/list.h"
#include "utils/oidc_error.h"

#include <stdarg.h>
#include <time.h>

struct connection* ipc_readAsyncFromMultipleConnections(struct connection,
                                                        list_t*);
char*              ipc_communicateWithPath(char*, ...);
char*              ipc_vcommunicateWithPath(char*, va_list);

oidc_error_t ipc_server_init(struct connection* con, const char* env_var_name);
oidc_error_t ipc_initWithPath(struct connection* con);
int          ipc_bindAndListen(struct connection* con);

void         server_ipc_freeLastKey();
char*        server_ipc_read(const int);
oidc_error_t server_ipc_write(const int, char*, ...);
oidc_error_t server_ipc_writeOidcErrno(const int);

#endif  // IPC_SERVER_H
