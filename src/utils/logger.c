#ifndef MACOS
#include <syslog.h>
#include <stdarg.h>

void logger_open(const char* logger_name){
  openlog(logger_name, LOG_CONS | LOG_PID, LOG_AUTHPRIV);
}


void logger(int log_level, const char* msg,...){
  va_list args;
  va_start(args, msg);
  vsyslog(LOG_AUTHPRIV | log_level, msg, args);
}

#else
#endif

