#include "agentLogger.h"

#include <stdio.h>

void (*agent_log)(int log_level, const char* msg, ...);

void setLogWithTerminal() { agent_log = loggerTerminal; }
void setWithoutTerminal() { agent_log = logger; };

void agent_openlog(const char* logger_name) {
  if (agent_log == NULL) {
    setWithoutTerminal();
  }
  logger_open(logger_name);
}
