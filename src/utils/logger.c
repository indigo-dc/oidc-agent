#include "logger.h"
#include <stdarg.h>

#ifdef __linux__
#include <syslog.h>

void logger_open(const char* logger_name){
  openlog(logger_name, LOG_CONS | LOG_PID, LOG_AUTHPRIV);
}


void logger(int log_level, const char* msg,...){
  va_list args;
  va_start(args, msg);
  vsyslog(LOG_AUTHPRIV | log_level, msg, args);
}

int logger_setlogmask(int mask) {
  return setlogmask(mask);
}

int logger_setloglevel(int level) {
  return setlogmask(LOG_UPTO(level));
}


#elif __APPLE__
#include <os/log.h>
#include "utils/stringUtils.h"
#include "utils/memory.h"

static os_log_t os_logger;
static char* os_logger_name;
static int os_log_level = DEBUG;


#include <time.h>
#include "utils/file_io/file_io.h"

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

void own_log(int log_level, const char* msg, va_list args){
    char* logmsg = oidc_vsprintf(msg, args);
    char* time_str = format_time();
    const char* const fmt = "%s %s %s: %s";
      const char* level;
    switch(log_level){
      case DEBUG: level="DEBUG";
                              break;
      case INFO: level="INFO";
                             break;
      case NOTICE: level="NOTICE";break;
      case WARNING: level="WARNING";break;
      case ERROR: level="ERROR";
                              break;
      case ALERT: level="ALERT";break;
      case EMERGENCY: level="EMERG";
                              break;
      default: level = "";
               break;
    }
    char* log = oidc_sprintf(fmt, time_str, os_logger_name, level, logmsg);
    secFree(time_str);
    secFree(logmsg);
    appendFile(os_logger_name, log);
    secFree(log);
}

void logger_open(const char* logger_name){
  // os_logger = os_log_create("edu.kit.data.oidc-agent", logger_name);
  os_logger_name = oidc_strcopy(logger_name);
}

void logger(int log_level, const char* msg,...){
  if(log_level>=os_log_level){
  va_list args;
  va_start(args, msg);
  //   char* logmsg = oidc_vsprintf(msg, args);
  // os_log_with_type(os_logger, log_level,"%s", logmsg);
  // secFree(logmsg);
own_log(log_level, msg, args);
  va_end(args);
  }
}

int logger_setlogmask(int mask){
  return logger_setloglevel(mask);
}

int logger_setloglevel(int level){
  int old = os_log_level;
  os_log_level=level;
  return old;
}
#endif

