#ifdef __linux__
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#endif
#include "logger.h"

#include <stdarg.h>

#ifdef __linux__
#include <syslog.h>

void logger_open(const char* logger_name) {
  openlog(logger_name, LOG_CONS | LOG_PID, LOG_AUTHPRIV);
}

void logger(int log_level, const char* msg, ...) {
  va_list args;
  va_start(args, msg);
  vsyslog(LOG_AUTHPRIV | log_level, msg, args);
}

int logger_setlogmask(int mask) { return setlogmask(mask); }

int logger_setloglevel(int level) { return setlogmask(LOG_UPTO(level)); }

#elif __APPLE__
#include "utils/memory.h"
#include "utils/stringUtils.h"

#include <time.h>
#include "utils/file_io/oidc_file_io.h"

static char* logger_name;
static int   log_level = NOTICE;

char* format_time() {
  char* s = secAlloc(sizeof(char) * (19 + 1));
  if (s == NULL) {
    return NULL;
  }
  time_t     now = time(NULL);
  struct tm* t   = localtime(&now);
  strftime(s, 19 + 1, "%F %H:%M:%S", t);
  return s;
}

void own_log(int _log_level, const char* msg, va_list args) {
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
  appendOidcFile("oidc-agent.log", log);
  secFree(log);
}

void logger_open(const char* _logger_name) {
  logger_name = oidc_strcopy(_logger_name);
}

void logger(int _log_level, const char* msg, ...) {
  if (_log_level >= log_level) {
    va_list args;
    va_start(args, msg);
    own_log(_log_level, msg, args);
    va_end(args);
  }
}

int logger_setlogmask(int mask) { return logger_setloglevel(mask); }

int logger_setloglevel(int level) {
  int old   = log_level;
  log_level = level;
  return old;
}
#endif
