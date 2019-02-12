#ifndef PROMPT_UTILS_H
#define PROMPT_UTILS_H

char* getEncryptionPassword(const char* forWhat, const char* suggestedPassword,
                            unsigned int max_pass_tries);

#endif  // PROMPT_UTILS_H
