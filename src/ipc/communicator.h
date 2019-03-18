#ifndef IPC_COMMUNICATER_H
#define IPC_COMMUNICATER_H

#include <stdarg.h>

char* ipc_communicate(const char* fmt, ...);
char* ipc_vcommunicate(const char* fmt, va_list args);

#endif  // IPC_COMMUNICATER_H
