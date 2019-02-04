#define _XOPEN_SOURCE 700

#include "prompt.h"
#include "memory.h"
#include "oidc_error.h"
#include "printer.h"
#include "utils/file_io/file_io.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <termios.h>
#include <unistd.h>

/** @fn char* promptPassword(char* prompt_str, ...)
 * @brief prompts the user and disables terminal echo for the userinput, so it
 * is useable for password prompts
 * @param prompt_str the prompt message to be displayed
 * @param ... if prompt_str is a fromat string, additional parameters can be
 * passed
 * @return a pointer to the user input. Has to be freed after usage.
 */
char* promptPassword(char* prompt_str, ...) {
  struct termios oflags, nflags;

  /* disabling echo */
  tcgetattr(STDIN_FILENO, &oflags);
  nflags = oflags;
  nflags.c_lflag &= ~ECHO;
  nflags.c_lflag |= ECHONL;

  if (tcsetattr(STDIN_FILENO, TCSANOW, &nflags) != 0) {
    syslog(LOG_AUTHPRIV | LOG_ERR, "tcsetattr: %m");
    oidc_errno = OIDC_ETCS;
    return NULL;
  }
  va_list args;
  va_start(args, prompt_str);
  char* msg = oidc_vsprintf(prompt_str, args);
  va_end(args);

  char* password = prompt(msg);
  secFree(msg);

  /* restore terminal */
  if (tcsetattr(STDIN_FILENO, TCSANOW, &oflags) != 0) {
    syslog(LOG_AUTHPRIV | LOG_ERR, "tcsetattr: %m");
    oidc_errno = OIDC_ETCS;
    return NULL;
  }

  return password;
}

/** @fn char* prompt(char* prompt_str, ...)
 * @brief prompts the user for userinput, should not be used for password
 * prompts, user \f promptPassword instead
 * @param prompt_str the prompt message to be displayed
 * @param ... if prompt_str is a fromat string, additional parameters can be
 * passed
 * @return a pointer to the user input. Has to freed after usage.
 */
char* prompt(char* prompt_str, ...) {
  va_list args;
  va_start(args, prompt_str);
  char* msg = oidc_vsprintf(prompt_str, args);
  va_end(args);

  printPrompt("%s", msg);
  secFree(msg);
  return getLineFromFILE(stdin);
}

int promptConsentDefaultNo(char* prompt_str) {
  char* res = prompt("%s %s", prompt_str, "[No/yes/quit]: ");
  if (strcmp(res, "yes") == 0) {
    secFree(res);
    return 1;
  } else if (strcmp(res, "quit") == 0) {
    exit(EXIT_SUCCESS);
  } else {
    secFree(res);
    return 0;
  }
}

int promptConsentDefaultYes(char* prompt_str) {
  char* res = prompt("%s %s", prompt_str, "[Yes/no/quit]: ");
  if (strcmp(res, "no") == 0) {
    secFree(res);
    return 0;
  } else if (strcmp(res, "quit") == 0) {
    exit(EXIT_SUCCESS);
  } else {
    secFree(res);
    return 1;
  }
}
