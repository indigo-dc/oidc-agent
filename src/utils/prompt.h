#ifndef PROMPT_H
#define PROMPT_H

char* promptPassword(char* prompt_str, ...);
char* prompt(char* prompt_str, ...);
int   promptConsentDefaultNo(char* prompt_str);
int   promptConsentDefaultYes(char* prompt_str);

#endif  // PROMPT_H
