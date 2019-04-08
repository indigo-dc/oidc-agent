#ifndef OIDC_LOGGER_H
#define OIDC_LOGGER_H

#ifndef MACOS
#include <syslog.h>
#define DEBUG LOG_DEBUG
#define INFO LOG_INFO
#define NOTICE LOG_NOTICE
#define WARNING LOG_WARNING
#define ERROR LOG_ERR
#define ALERT LOG_ALERT
#define EMERGENCY LOG_EMERG
#endif

void logger_open(const char* logger_name);
void logger(int log_level, const char* msg,...);

#endif // OIDC_LOGGER_H
