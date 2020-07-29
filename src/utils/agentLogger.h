#ifndef OIDC_AGENT_LOGGER_H
#define OIDC_AGENT_LOGGER_H

#include "utils/logger.h"

void setLogWithTerminal();
void setLogWithoutTerminal();

void (*agent_log)(int log_level, const char* msg, ...);
void agent_openlog(const char* logger_name);

#endif /* OIDC_AGENT_LOGGER_H */
