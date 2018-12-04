#ifndef IPC_CRYPT_H
#define IPC_CRYPT_H

#include "utils/oidc_error.h"

#include <stdarg.h>

oidc_error_t server_ipc_vcryptWrite(int sock, const unsigned char* key,
                                    char* fmt, va_list args);
oidc_error_t server_ipc_cryptWrite(int sock, const unsigned char* key,
                                   char* fmt, ...);

oidc_error_t server_ipc_write(int sock, char* fmt, ...);
char*        server_ipc_read(int sock);
oidc_error_t server_ipc_writeOidcErrno(int sock);

#endif  // IPC_CRYPT_H
