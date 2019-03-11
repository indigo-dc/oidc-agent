#ifndef CRYPT_COMMUNICATOR_H
#define CRYPT_COMMUNICATOR_H

#include <stdarg.h>

char* ipc_cryptCommunicate(const char*, ...);
char* ipc_vcryptCommunicate(const char*, va_list);
char* ipc_vcryptCommunicateWithPath(const char*, const char*, va_list);
char* ipc_cryptCommunicateWithPath(const char*, const char*, ...);

#endif  // CRYPT_COMMUNICATOR_H
