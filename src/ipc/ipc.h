#ifndef IPC_H
#define IPC_H

#include "connection.h"
#ifdef __MSYS__
#include <winsock2.h>
#else
#include "socket.h"
#endif
#include "utils/oidc_error.h"

#include <stdarg.h>
#include <time.h>

#ifndef __MSYS__
oidc_error_t initConnectionWithoutPath(struct connection*, int, int);
oidc_error_t initConnectionWithPath(struct connection*, const char*);
#endif
oidc_error_t ipc_client_init(struct connection*, unsigned char);

int ipc_connect(struct connection con);

char* ipc_read(const SOCKET _sock);
#ifndef __MSYS__
char* ipc_readWithTimeout(const SOCKET _sock, time_t timeout);
#endif

oidc_error_t ipc_write(SOCKET _sock, const char* msg, ...);
oidc_error_t ipc_vwrite(SOCKET _sock, const char* msg, va_list args);
oidc_error_t ipc_writeOidcErrno(SOCKET sock);

int          ipc_close(SOCKET _sock);
oidc_error_t ipc_closeConnection(struct connection* con);

char* ipc_communicateWithSock(SOCKET sock, const char* fmt, ...);
char* ipc_vcommunicateWithSock(SOCKET sock, const char* fmt, va_list args);

struct timeval* initTimeout(time_t death);

#endif  // IPC_H
