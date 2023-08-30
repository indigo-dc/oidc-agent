#ifndef OIDC_AGENT_LOGGER_H
#define OIDC_AGENT_LOGGER_H

#include "utils/logger.h"

void          setLogWithTerminal();
unsigned char logWithTerminal();

#define agent_log(LOG_LEVEL, MSG, ...)                              \
  logWithTerminal() ? loggerTerminal(LOG_LEVEL, MSG, ##__VA_ARGS__) \
                    : logger(LOG_LEVEL, MSG, ##__VA_ARGS__)

void agent_openlog(const char* logger_name);

#endif /* OIDC_AGENT_LOGGER_H */
