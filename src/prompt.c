#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <syslog.h>

#include "prompt.h"

/** @fn char* promptPassword(char* prompt_str)
 * @brief prompts the user and disables terminal echo for the userinput, so it
 * is useable for password prompts
 * @param prompt_str the prompt message to be displayed
 * @return a pointer to the user input. Has to freed after usage.
 */
char* promptPassword(char* prompt_str) {
  struct termios oflags, nflags;

  /* disabling echo */
  tcgetattr(STDIN_FILENO, &oflags);
  nflags = oflags;
  nflags.c_lflag &= ~ECHO;
  nflags.c_lflag |= ECHONL;

  if (tcsetattr(STDIN_FILENO, TCSANOW, &nflags) != 0) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "tcsetattr: %m");
    return NULL;
  }

  char* password = prompt(prompt_str);

  /* restore terminal */
  if (tcsetattr(STDIN_FILENO, TCSANOW, &oflags) != 0) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "tcsetattr: %m");
    return NULL;
  }

  return password;
}

/** @fn char* prompt(char* prompt_str)
 * @brief prompts the user for userinput, should not be used for password
 * prompts, user \f promptPassword instead
 * @param prompt_str the prompt message to be displayed
 * @return a pointer to the user input. Has to freed after usage.
 */
char* prompt(char* prompt_str) {
  printf("%s", prompt_str);
  char* buf = NULL;
  size_t len = 0;
  int n;
  if ((n = getline(&buf, &len, stdin))<0) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "getline: %m");
    return NULL; 
  }
  buf[n-1] = 0; //removing '\n'
  return buf;
}
