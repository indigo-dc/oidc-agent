#include "gen_signal_handler.h"
#include "defines/ipc_values.h"
#include "ipc/cryptCommunicator.h"
#include "utils/memory.h"
#include "utils/stringUtils.h"

#include <signal.h>
#include <stddef.h>
#include <stdlib.h>
#include <syslog.h>

static char*          global_state = NULL;
static __sighandler_t old_sigint;

void gen_http_signal_handler(int signo) {
  switch (signo) {
    case SIGINT:
      if (global_state) {
        _secFree(ipc_cryptCommunicate(REQUEST_TERMHTTP, global_state));
        secFree(global_state);
        global_state = NULL;
      }
      break;
    default:
      syslog(LOG_AUTHPRIV | LOG_EMERG, "HttpServer caught Signal %d", signo);
  }
  exit(signo);
}

void registerSignalHandler(const char* state) {
  global_state = oidc_sprintf(state);
  old_sigint   = signal(SIGINT, gen_http_signal_handler);
}

void unregisterSignalHandler() {
  secFree(global_state);
  global_state = NULL;
  signal(SIGINT, old_sigint);
}
