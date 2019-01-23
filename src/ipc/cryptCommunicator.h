#ifndef CRYPT_COMMUNICATOR_H
#define CRYPT_COMMUNICATOR_H

#include <stdarg.h>

char* ipc_cryptCommunicate(char* fmt, ...);
char* ipc_vcryptCommunicate(char* fmt, va_list args);

#endif  // CRYPT_COMMUNICATOR_H
