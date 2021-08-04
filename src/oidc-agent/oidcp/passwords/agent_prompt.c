#include "agent_prompt.h"

#include <signal.h>

#include "utils/prompt.h"

typedef void (*sighandler_t)(int);

char* agent_promptPassword(const char* text, const char* label,
                           const char* init) {
  // _promptPasswordGUI might raise SIGINT (if user cancels), oidcp should not
  // crash then
  sighandler_t old = signal(SIGINT, SIG_IGN);
  char*        ret = _promptPasswordGUI(text, label, init);
  signal(SIGINT, old);
  return ret;
}

int agent_promptConsentDefaultYes(const char* text) {
  return _promptConsentGUIDefaultYes(text);
}
