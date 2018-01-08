#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <syslog.h>
#include <stdlib.h>
#include <stdarg.h>

#include "prompt.h"
#include "oidc_utilities.h"
#include "oidc_error.h"

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

  if(tcsetattr(STDIN_FILENO, TCSANOW, &nflags) != 0) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "tcsetattr: %m");
    oidc_errno = OIDC_ETCS;
    return NULL;
  }
  va_list args, original;
  va_start(original, prompt_str);
  va_start(args, prompt_str);
  char* msg = calloc(sizeof(char), vsnprintf(NULL, 0, prompt_str, args)+1);
  vsprintf(msg, prompt_str, original);

  char* password = prompt(msg);
  clearFreeString(msg);

  /* restore terminal */
  if(tcsetattr(STDIN_FILENO, TCSANOW, &oflags) != 0) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "tcsetattr: %m");
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
  char* msg = calloc(sizeof(char), vsnprintf(NULL, 0, prompt_str, args)+1);
  vsprintf(msg, prompt_str, original);

  printf("%s", msg);
  clearFreeString(msg);
  char* buf = NULL;
  size_t len = 0;
  int n;
  if((n = getline(&buf, &len, stdin))<0) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "getline: %m");
    oidc_errno = OIDC_EIN;
    return NULL; 
  }
  buf[n-1] = 0; //removing '\n'
  return buf;
}

int getUserConfirmation(char* prompt_str) {
  char* res = prompt("%s %s", prompt_str, "[yes/no/quit]: ");
  if(strcmp(res, "yes")==0) {
    clearFreeString(res);
    return 1;
  } else if(strcmp(res, "quit")==0) {
    exit(EXIT_SUCCESS);
  } else {
    clearFreeString(res);
    return 0;
  }
}
