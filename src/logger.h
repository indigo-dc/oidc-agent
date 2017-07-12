#ifndef LOGGER_H
#define LOGGER_H

#include <stdarg.h>

#define LOG_LEVEL 0

enum {ERROR, WARNING, INFO, DEBUG};

void logging(int level, const char* message, ...) ;

#endif //LOGGER_H

