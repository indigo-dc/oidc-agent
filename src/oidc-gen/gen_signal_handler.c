#define _GNU_SOURCE
#include "gen_signal_handler.h"

#include <signal.h>
#include <stddef.h>
#include <stdlib.h>

#include "defines/ipc_values.h"
#include "ipc/cryptCommunicator.h"
#include "utils/logger.h"
#include "utils/memory.h"
#include "utils/string/stringUtils.h"

static char* global_state = NULL;
#ifndef __APPLE__
static sighandler_t old_sigint;
#else
static sig_t old_sigint;
#endif

void gen_http_signal_handler(int signo) {
  switch (signo) {
    case SIGINT:
      if (global_state) {
        _secFree(ipc_cryptCommunicate(0, REQUEST_TERMHTTP, global_state));
        secFree(global_state);
        global_state = NULL;
      }
      break;
    default: logger(EMERGENCY, "oidc-gen caught Signal %d", signo);
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
