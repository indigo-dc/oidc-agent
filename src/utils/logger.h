#ifndef OIDC_LOGGER_H
#define OIDC_LOGGER_H

#ifdef __linux__

#include <syslog.h>
#define DEBUG LOG_DEBUG
#define INFO LOG_INFO
#define NOTICE LOG_NOTICE
#define WARNING LOG_WARNING
#define ERROR LOG_ERR
#define ALERT LOG_ALERT
#define EMERGENCY LOG_EMERG

#else

#define DEBUG 1
#define INFO 2
#define NOTICE 3
#define WARNING 4
#define ERROR 5
#define ALERT 6
#define EMERGENCY 7

#endif

#define _S1(x) #x
#define _S2(x) _S1(x)
#define LOCATION "(" __FILE__ ":" _S2(__LINE__) ") "
#define LOG_FMT(x) LOCATION x

void logger_open(const char* logger_name);
void _logger(int log_level, const char* msg, ...);
void _loggerTerminal(int log_level, const char* msg, ...);
int  logger_setlogmask(int);
int  logger_setloglevel(int);
#define logger(LOG_LEVEL, MSG, ...) \
  _logger(LOG_LEVEL, LOG_FMT(MSG), ##__VA_ARGS__)
#define loggerTerminal(LOG_LEVEL, MSG, ...) \
  _loggerTerminal(LOG_LEVEL, LOG_FMT(MSG), ##__VA_ARGS__)

#endif  // OIDC_LOGGER_H
