#ifndef CRYPT_COMMUNICATOR_H
#define CRYPT_COMMUNICATOR_H

#include <stdarg.h>

#include "defines/msys.h"

char* ipc_cryptCommunicate(unsigned char, const char*, ...);
char* ipc_vcryptCommunicate(unsigned char, const char*, va_list);
#ifndef MINGW
char* ipc_vcryptCommunicateWithPath(const char*, const char*, va_list);
char* ipc_cryptCommunicateWithPath(const char*, const char*, ...);
#endif
#endif  // CRYPT_COMMUNICATOR_H
