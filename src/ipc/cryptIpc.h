#ifndef IPC_CRYPT_H
#define IPC_CRYPT_H

#include "utils/oidc_error.h"

#include <stdarg.h>

char*        server_ipc_readFromSocket(int sock);
char*        server_ipc_read(int rx, int tx);
char*        server_ipc_cryptRead(int rx, int tx, const char* msg);
oidc_error_t server_ipc_write(int tx, char* fmt, ...);
oidc_error_t server_ipc_vcryptWrite(int tx, const unsigned char* key, char* fmt,
                                    va_list args);
oidc_error_t server_ipc_cryptWrite(int tx, const unsigned char* key, char* fmt,
                                   ...);
oidc_error_t server_ipc_writeOidcErrno(int tx);

#endif  // IPC_CRYPT_H
