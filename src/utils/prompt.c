#define _XOPEN_SOURCE 700

#include "prompt.h"
#include "memory.h"
#include "oidc_error.h"
#include "printer.h"

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
  va_list args, original;
  va_start(original, prompt_str);
  va_start(args, prompt_str);
  char* msg =
      secAlloc(sizeof(char) * (vsnprintf(NULL, 0, prompt_str, args) + 1));
  vsprintf(msg, prompt_str, original);

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
  va_list args, original;
  va_start(original, prompt_str);
  va_start(args, prompt_str);
  char* msg =
      secAlloc(sizeof(char) * (vsnprintf(NULL, 0, prompt_str, args) + 1));
  vsprintf(msg, prompt_str, original);

  printPrompt("%s", msg);
  secFree(msg);
  char*  buf = NULL;
  size_t len = 0;
  int    n;
  if ((n = getline(&buf, &len, stdin)) < 0) {
    syslog(LOG_AUTHPRIV | LOG_ERR, "getline: %m");
    oidc_errno = OIDC_EIN;
    return NULL;
  }
  buf[n - 1] = 0;  // removing '\n'
  char* secFreeAblePointer =
      oidc_strcopy(buf);  // Because getline allocates memory using malloc and
                          // not secAlloc, we cannot free buf with secFree. To
                          // be able to do so we copy the buf to memory
                          // allocated with secAlloc and free buf using secFreeN
  secFreeN(buf, n);
  return secFreeAblePointer;
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
