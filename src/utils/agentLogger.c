#include "agentLogger.h"

#include <stdio.h>

static unsigned char _logWithTerminal = 0;

void          setLogWithTerminal() { _logWithTerminal = 1; }
unsigned char logWithTerminal() { return _logWithTerminal; }

void agent_openlog(const char* logger_name) { logger_open(logger_name); }
