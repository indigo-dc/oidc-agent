#ifndef IPC_TCP_SERVER_H
#define IPC_TCP_SERVER_H

#include "connection.h"
#include "utils/oidc_error.h"

#include <stdarg.h>
#include <time.h>

oidc_error_t ipc_tcp_server_init(struct connection* con, unsigned short port);
int          ipc_tcp_bindAndListen(struct connection* con);

#endif  // IPC_TCP_SERVER_H
