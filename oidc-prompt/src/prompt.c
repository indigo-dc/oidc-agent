#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>

#include "prompt.h"

char* promptPassword(char* prompt_str) {
  struct termios oflags, nflags;

  /* disabling echo */
  tcgetattr(fileno(stdin), &oflags);
  nflags = oflags;
  nflags.c_lflag &= ~ECHO;
  nflags.c_lflag |= ECHONL;

  if (tcsetattr(fileno(stdin), TCSANOW, &nflags) != 0) {
    perror("tcsetattr");
    return NULL;
  }

  char* password = prompt(prompt_str);

  /* restore terminal */
  if (tcsetattr(fileno(stdin), TCSANOW, &oflags) != 0) {
    perror("tcsetattr");
    return NULL;
  }

  return password;
}

char* prompt(char* prompt_str) {
  printf("%s", prompt_str);
  char* buf = NULL;
  size_t len = 0;
  int n;
  if ((n = getline(&buf, &len, stdin))<0) {
    perror("getline");
    return NULL; 
  }
  buf[n-1] = 0; //removing '\n'
  return buf;
}
