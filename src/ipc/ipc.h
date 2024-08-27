#ifndef IPC_H
#define IPC_H

#include "connection.h"
#include "defines/msys.h"
#ifdef MINGW
#include <winsock2.h>
#else
#include "socket.h"
#endif
#include <stdarg.h>
#include <time.h>

#include "utils/oidc_error.h"

char* defaultSocketPath();

#ifndef MINGW
oidc_error_t initConnectionWithoutPath(struct connection*, int, int);
oidc_error_t initConnectionWithPath(struct connection*, const char*);
#endif
oidc_error_t ipc_client_init(struct connection*, unsigned char);

int ipc_connect(struct connection con);

char* ipc_read(const SOCKET _sock);
#ifndef MINGW
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

#ifdef MINGW
oidc_error_t ipc_msys_authorize(struct connection con);
#endif

#endif  // IPC_H
