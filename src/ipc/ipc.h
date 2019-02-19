#ifndef IPC_H
#define IPC_H

#include "connection.h"
#include "utils/oidc_error.h"

#include <stdarg.h>
#include <time.h>

oidc_error_t initConnectionWithoutPath(struct connection*, int);
oidc_error_t initConnectionWithPath(struct connection*, const char*);
oidc_error_t ipc_client_init(struct connection*, const char*);

int ipc_connect(struct connection con);

char* ipc_read(const int _sock);
char* ipc_readWithTimeout(const int _sock, time_t timeout);

oidc_error_t ipc_write(int _sock, char* msg, ...);
oidc_error_t ipc_vwrite(int _sock, char* msg, va_list args);
oidc_error_t ipc_writeOidcErrno(int sock);

int          ipc_close(int _sock);
oidc_error_t ipc_closeConnection(struct connection* con);
oidc_error_t ipc_closeAndUnlinkConnection(struct connection* con);

char* ipc_communicateWithSock(int sock, char* fmt, ...);
char* ipc_vcommunicateWithSock(int sock, char* fmt, va_list args);

struct timeval* initTimeout(time_t death);

#endif  // IPC_H
