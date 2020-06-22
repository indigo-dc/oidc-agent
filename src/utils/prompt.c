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

char* _promptPasswordGUI(const char* prompt) {
  char* cmd = oidc_sprintf("ssh-askpass \"%s\"", prompt);
  char* ret = getOutputFromCommand(cmd);
  secFree(cmd);
  return ret;
}

char* _promptGUI(const char* prompt) { return _promptPasswordGUI(prompt); }

char* _promptCLI(const char* prompt) {
  printPrompt("%s", prompt);
  return getLineFromFILE(stdin);
}

char* _promptPasswordCLI(const char* prompt) {
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

  char* password = _promptCLI(prompt);

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
 * @param prompt_str the prompt message to be displayed
 * @param ... if prompt_str is a fromat string, additional parameters can be
 * passed
 * @return a pointer to the user input. Has to be freed after usage.
 */
char* promptPassword(const char* prompt_str, ...) {
  va_list args;
  va_start(args, prompt_str);
  char* msg = oidc_vsprintf(prompt_str, args);
  va_end(args);
  char* password = NULL;
  switch (prompt_mode()) {
    case PROMPT_MODE_CLI: password = _promptPasswordCLI(msg); break;
    case PROMPT_MODE_GUI: password = _promptPasswordGUI(msg); break;
    default:
      logger(ERROR, "Invalid prompt mode");
      oidc_setInternalError("Programming error: Prompt Mode must be set!");
      oidc_perror();
      return NULL;
  }
  secFree(msg);
  return password;
}

/** @fn char* prompt(char* prompt_str, ...)
 * @brief prompts the user for userinput, should not be used for password
 * prompts, user \f promptPassword instead
 * @param prompt_mode the mode how the user should be prompted
 * @param prompt_str the prompt message to be displayed
 * @param ... if prompt_str is a fromat string, additional parameters can be
 * passed
 * @return a pointer to the user input. Has to freed after usage.
 */
char* prompt(const char* prompt_str, ...) {
  va_list args;
  va_start(args, prompt_str);
  char* msg = oidc_vsprintf(prompt_str, args);
  va_end(args);

  char* password = NULL;
  switch (prompt_mode()) {
    case PROMPT_MODE_CLI: password = _promptCLI(msg); break;
    case PROMPT_MODE_GUI: password = _promptGUI(msg); break;
    default:
      logger(ERROR, "Invalid prompt mode");
      oidc_setInternalError("Programming error: Prompt Mode must be set!");
      oidc_perror();
      return NULL;
  }
  secFree(msg);
  return password;
}

int _promptConsentGUI(const char* prompt_msg) {
  char* cmd = oidc_sprintf("ssh-askpass \"%s\"", prompt_msg);
  char* ret = getOutputFromCommand(cmd);
  secFree(cmd);
  return ret == NULL || strcaseequal(ret, "no") ? 0 : 1;
}

int _promptConsentGUIDefaultNo(const char* prompt_msg) {
  char* cmd = oidc_sprintf("ssh-askpass \"%s\"", prompt_msg);
  char* ret = getOutputFromCommand(cmd);
  secFree(cmd);
  return ret == NULL || !strcaseequal(ret, "yes") ? 0 : 1;
}

int promptConsentDefaultNo(const char* prompt_str) {
  if (prompt_mode() == PROMPT_MODE_GUI) {
    char* prompt =
        oidc_sprintf("%s\nType 'yes' to confirm. Anything else cancels.");
    int ret = _promptConsentGUIDefaultNo(prompt);
    secFree(prompt);
    return ret;
  }
  char* res = prompt("%s %s", prompt_str, "[No/yes/quit]: ");
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

int promptConsentDefaultYes(const char* prompt_str) {
  if (prompt_mode() == PROMPT_MODE_GUI) {
    char* prompt = oidc_sprintf(
        "%s\nType 'no' or cancel to cancel. Anything else confirms.");
    int ret = _promptConsentGUIDefaultNo(prompt);
    secFree(prompt);
    return ret;
  }
  char* res = prompt("%s %s", prompt_str, "[Yes/no/quit]: ");
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
