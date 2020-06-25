#define _XOPEN_SOURCE 700

#include "prompt.h"
#include "memory.h"
#include "oidc_error.h"
#include "printer.h"
#include "utils/file_io/file_io.h"
#include "utils/logger.h"
#include "utils/prompt_mode.h"
#include "utils/system_runner.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

char* _promptPasswordGUI(const char* text, const char* label,
                         const char* init) {
  char* cmd = oidc_sprintf("oidc-prompt password \"oidc-agent prompt\" \"%s\" "
                           "\"%s\" \"%s\"",
                           text, label, init ?: "");
  char* ret = getOutputFromCommand(cmd);
  secFree(cmd);
  return ret;
}

char* _promptGUI(const char* text, const char* label, const char* init) {
  char* cmd = oidc_sprintf(
      "oidc-prompt input \"oidc-agent prompt\" \"%s\" \"%s\" \"%s\"", text,
      label, init ?: "");
  char* ret = getOutputFromCommand(cmd);
  secFree(cmd);
  return ret;
}

char* _promptCLI(const char* text, const char* init) {
  printPrompt("%s", text);
  if (init) {
    printPrompt(" [%s]", init);
  }
  printPrompt(": ");
  return getLineFromFILE(stdin);
}

char* _promptPasswordCLI(const char* text, const char* init) {
  struct termios oflags, nflags;

  /* disabling echo */
  tcgetattr(STDIN_FILENO, &oflags);
  nflags = oflags;
  nflags.c_lflag &= ~ECHO;
  nflags.c_lflag |= ECHONL;

  if (tcsetattr(STDIN_FILENO, TCSANOW, &nflags) != 0) {
    logger(ERROR, "tcsetattr: %m");
    oidc_errno = OIDC_ETCS;
    return NULL;
  }

  char* password = _promptCLI(text, init ? "***" : NULL);

  /* restore terminal */
  if (tcsetattr(STDIN_FILENO, TCSANOW, &oflags) != 0) {
    logger(ERROR, "tcsetattr: %m");
    oidc_errno = OIDC_ETCS;
    return NULL;
  }

  return password;
}

/** @fn char* promptPassword(char* prompt_str, ...)
 * @brief prompts the user and disables terminal echo for the userinput, so it
 * is useable for password prompts
 * @param prompt_mode the mode how the user should be prompted
 * @return a pointer to the user input. Has to be freed after usage.
 */
char* promptPassword(const char* text, const char* label, const char* init,
                     unsigned char cliVerbose) {
  char* password = NULL;
  switch (pw_prompt_mode()) {
    case PROMPT_MODE_CLI:
      password = _promptPasswordCLI(cliVerbose ? text : label, init);
      break;
    case PROMPT_MODE_GUI:
      password = _promptPasswordGUI(text, label, init);
      break;
    default:
      logger(ERROR, "Invalid prompt mode");
      oidc_setInternalError("Programming error: Prompt Mode must be set!");
      oidc_perror();
      return NULL;
  }
  return password;
}

/** @fn char* prompt(char* prompt_str, ...)
 * @brief prompts the user for userinput, should not be used for password
 * prompts, user \f promptPassword instead
 * @param prompt_mode the mode how the user should be prompted
 * @return a pointer to the user input. Has to freed after usage.
 */
char* prompt(const char* text, const char* label, const char* init,
             unsigned char cliVerbose) {
  char* password = NULL;
  switch (prompt_mode()) {
    case PROMPT_MODE_CLI:
      password = _promptCLI(cliVerbose ? text : label, init);
      break;
    case PROMPT_MODE_GUI: password = _promptGUI(text, label, init); break;
    default:
      logger(ERROR, "Invalid prompt mode");
      oidc_setInternalError("Programming error: Prompt Mode must be set!");
      oidc_perror();
      return NULL;
  }
  return password;
}

int _promptConsentGUIDefaultYes(const char* text) {
  char* cmd = oidc_sprintf("oidc-prompt confirm-default-yes --title "
                           "\"oidc-agent prompt confirm\" \"%s\"",
                           text);
  char* ret = getOutputFromCommand(cmd);
  secFree(cmd);
  return ret != NULL && strcaseequal(ret, "yes") ? 1 : 0;
}

int _promptConsentGUIDefaultNo(const char* text) {
  char* cmd = oidc_sprintf(
      "oidc-prompt confirm-default-no \"oidc-agent prompt confirm\" \"%s\"",
      text);
  char* ret = getOutputFromCommand(cmd);
  secFree(cmd);
  return ret != NULL && strcaseequal(ret, "yes") ? 1 : 0;
}

int promptConsentDefaultNo(const char* text) {
  if (prompt_mode() == PROMPT_MODE_GUI) {
    return _promptConsentGUIDefaultNo(text);
  }
  char* res = prompt(text, NULL, "No/yes/quit", CLI_PROMPT_VERBOSE);
  if (strequal(res, "yes")) {
    secFree(res);
    return 1;
  } else if (strequal(res, "quit")) {
    exit(EXIT_SUCCESS);
  } else {
    secFree(res);
    return 0;
  }
}

int promptConsentDefaultYes(const char* text) {
  if (prompt_mode() == PROMPT_MODE_GUI) {
    return _promptConsentGUIDefaultNo(text);
  }
  char* res = prompt(text, NULL, "Yes/no/quit", CLI_PROMPT_VERBOSE);
  if (strequal(res, "no")) {
    secFree(res);
    return 0;
  } else if (strequal(res, "quit")) {
    exit(EXIT_SUCCESS);
  } else {
    secFree(res);
    return 1;
  }
}
