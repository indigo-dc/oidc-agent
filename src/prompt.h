#ifndef PROMPT_H
#define PROMPT_H

char* promptPassword(char* prompt_str, ...);
char* prompt(char* prompt_str, ...);
int   getUserConfirmation(char* prompt_str);

#endif  // PROMPT_H
