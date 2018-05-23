#ifndef IPC_COMMUNICATER_H
#define IPC_COMMUNICATER_H

#include <stdarg.h>

char* ipc_communicate(char* fmt, ...) ;
char* ipc_vcommunicate(char* fmt, va_list args) ;

char* ipc_communicateWithPath(char* fmt, ...) ;
char* ipc_vcommunicateWithPath(char* fmt, va_list args) ;

#endif // IPC_COMMUNICATER_H
