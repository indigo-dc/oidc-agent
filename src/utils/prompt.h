#ifndef PROMPT_H
#define PROMPT_H

#include "wrapper/list.h"

#define CLI_PROMPT_NOT_VERBOSE 0
#define CLI_PROMPT_VERBOSE 1

typedef char* (*promptFnc)(const char*, const char*, const char*,
                           unsigned char);

char* _promptPasswordGUI(const char* text, const char* label, const char* init);
int   _promptConsentGUIDefaultYes(const char* text);
char* promptPassword(const char* text, const char* label, const char* init,
                     unsigned char cliVerbose);
char* prompt(const char* text, const char* label, const char* init,
             unsigned char cliVerbose);
int   promptConsentDefaultNo(const char* text);
int   promptConsentDefaultYes(const char* text);
list_t* promptMultiple(const char* text, const char* label, list_t* init,
                       unsigned char cliVerbose);
char*   promptSelect(const char* text, const char* label, list_t* options,
                     size_t favPos, unsigned char cliVerbose);

#endif  // PROMPT_H
