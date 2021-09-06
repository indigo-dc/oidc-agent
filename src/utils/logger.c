#define _POSIX_C_SOURCE 200809L
#ifdef __linux__
#define _BSD_SOURCE
#define _DEFAULT_SOURCE
#endif
#include "logger.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/stringUtils.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static const char* logger_name;

char* format_time() {
  char* s = secAlloc(sizeof(char) * (19 + 1));
  if (s == NULL) {
    return NULL;
  }
  time_t     now = time(NULL);
  struct tm* t   = secAlloc(sizeof(struct tm));
  if (localtime_r(&now, t) == NULL) {
    oidc_perror();
    secFree(t);
    return NULL;
  }
  strftime(s, 19 + 1, "%F %H:%M:%S", t);
  secFree(t);
  return s;
}

char* create_log_message(int _log_level, const char* msg, va_list args) {
  char*             logmsg   = oidc_vsprintf(msg, args);
  char*             time_str = format_time();
  const char* const fmt      = "%s %s %s: %s";
  const char*       level;
  switch (_log_level) {
    case DEBUG: level = "DEBUG"; break;
    case INFO: level = "INFO"; break;
    case NOTICE: level = "NOTICE"; break;
    case WARNING: level = "WARNING"; break;
    case ERROR: level = "ERROR"; break;
    case ALERT: level = "ALERT"; break;
    case EMERGENCY: level = "EMERG"; break;
    default: level = ""; break;
  }
  char* log = oidc_sprintf(fmt, time_str, logger_name, level, logmsg);
  secFree(time_str);
  secFree(logmsg);
  return log;
}

#ifdef __linux__
#include <syslog.h>

static int _mask;

void logger_open(const char* _logger_name) {
  openlog(_logger_name, LOG_CONS | LOG_PID, LOG_AUTHPRIV);
  logger_name = _logger_name;
}

void logger(int log_level, const char* msg, ...) {
  va_list args;
  va_start(args, msg);
  vsyslog(LOG_AUTHPRIV | log_level, msg, args);
}

void loggerTerminal(int log_level, const char* msg, ...) {
  va_list args, copy;
  va_start(args, msg);
  va_copy(copy, args);
  vsyslog(LOG_AUTHPRIV | log_level, msg, args);
  if (_mask & LOG_MASK(log_level)) {
    char* logmsg = create_log_message(log_level, msg, copy);
    fprintf(stderr, "%s\n", logmsg);
    secFree(logmsg);
  }
}

int logger_setlogmask(int mask) {
  _mask = mask;
  return setlogmask(mask);
}

int logger_setloglevel(int level) { return logger_setlogmask(LOG_UPTO(level)); }

#else

#include "utils/file_io/oidc_file_io.h"

static int log_level = NOTICE;

void own_log(int terminal, int _log_level, const char* msg, va_list args) {
  char* log = create_log_message(_log_level, msg, args);
  appendOidcFile("oidc-agent.log", log);
  if (terminal) {
    fprintf(stderr, "%s\n", log);
  }
  secFree(log);
}

void logger_open(const char* _logger_name) {
  logger_name = oidc_strcopy(_logger_name);
}

void _logger(int terminal, int _log_level, const char* msg, va_list args) {
  if (_log_level >= log_level) {
    own_log(terminal, _log_level, msg, args);
  }
}

void loggerTerminal(int _log_level, const char* msg, ...) {
  va_list args;
  va_start(args, msg);
  _logger(1, _log_level, msg, args);
  va_end(args);
}

void logger(int _log_level, const char* msg, ...) {
  va_list args;
  va_start(args, msg);
  _logger(0, _log_level, msg, args);
  va_end(args);
}

int logger_setlogmask(int mask) { return logger_setloglevel(mask); }

int logger_setloglevel(int level) {
  int old   = log_level;
  log_level = level;
  return old;
}
#endif
