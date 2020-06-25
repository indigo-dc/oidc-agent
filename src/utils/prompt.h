#ifndef PROMPT_H
#define PROMPT_H

#define CLI_PROMPT_VERBOSE 1

char* _promptPasswordGUI(const char* text, const char* label, const char* init);
int   _promptConsentGUIDefaultYes(const char* text);
char* promptPassword(const char* text, const char* label, const char* init,
                     unsigned char cliVerbose);
char* prompt(const char* text, const char* label, const char* init,
             unsigned char cliVerbose);
int   promptConsentDefaultNo(const char* text);
int   promptConsentDefaultYes(const char* text);

#endif  // PROMPT_H
