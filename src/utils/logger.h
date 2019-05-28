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

#elif __APPLE__

#define DEBUG 1
#define INFO 2
#define NOTICE 3
#define WARNING 4
#define ERROR 5
#define ALERT 6
#define EMERGENCY 7

#endif

void logger_open(const char* logger_name);
void logger(int log_level, const char* msg,...);
int logger_setlogmask(int);
int logger_setloglevel(int);

#endif // OIDC_LOGGER_H
