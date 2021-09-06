#ifndef IPC_H
#define IPC_H

#include "connection.h"
#include "utils/oidc_error.h"

#include <stdarg.h>
#include <time.h>

oidc_error_t ipc_client_init(struct connection*, unsigned char remote);
oidc_error_t ipc_write(SOCKET _sock, const char* msg, ...);
oidc_error_t ipc_vwrite(SOCKET _sock, const char* msg, va_list args);
char* ipc_read(const SOCKET _sock);

oidc_error_t ipc_connect(struct connection con);
oidc_error_t ipc_msys_authorize(struct connection con);

char* ipc_communicateWithSock(SOCKET sock, const char* fmt, ...);
char* ipc_vcommunicateWithSock(SOCKET sock, const char* fmt, va_list args);

int          ipc_close(SOCKET _sock);
oidc_error_t ipc_closeConnection(struct connection* con);


#endif  // IPC_H
