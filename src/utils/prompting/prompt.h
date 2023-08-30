#ifndef PROMPT_H
#define PROMPT_H

#include "wrapper/list.h"

#define CLI_PROMPT_NOT_VERBOSE 0
#define CLI_PROMPT_VERBOSE 1

#define PROMPT_NO_TIMEOUT 0
#define PROMPT_DEFAULT_TIMEOUT 300

typedef char* (*promptFnc)(const char*, const char*, const char*,
                           unsigned char);

char* _promptPasswordGUI(const char* text, const char* label, const char* init,
                         const int timeout);
int   _promptConsentGUIDefaultYes(const char* text, const int timeout);
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
void    displayLinkGUI(const char* text, const char* link, const char* qr_path);
char*   promptMytokenConsentGUI(const char* base64html, const int timeout);

#endif  // PROMPT_H
