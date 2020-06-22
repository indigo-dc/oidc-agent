#ifndef PROMPT_H
#define PROMPT_H

char* _promptPasswordGUI(const char* prompt);
int   _promptConsentGUI(const char* prompt_msg);
char* promptPassword(const char* prompt_str, ...);
char* prompt(const char* prompt_str, ...);
int   promptConsentDefaultNo(const char* prompt_str);
int   promptConsentDefaultYes(const char* prompt_str);

#endif  // PROMPT_H
