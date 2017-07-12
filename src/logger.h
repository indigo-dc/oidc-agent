#ifndef LOGGER_H
#define LOGGER_H

#include <stdarg.h>

#define LOG_LEVEL 3

static const char* levels[] = {"ERROR","WARNING","INFO","DEBUG"};

void logging(int level, const char* message, ...) ;

#endif //LOGGER_H

