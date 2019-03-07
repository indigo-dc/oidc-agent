#ifndef PROMPT_UTILS_H
#define PROMPT_UTILS_H

char* getEncryptionPasswordFor(const char* forWhat,
                               const char* suggestedPassword,
                               const char* pw_cmd);
char* getEncryptionPasswordForAccountConfig(const char* shortname,
                                            const char* suggestedPassword,
                                            const char* pw_cmd);
char* getDecryptionPasswordFor(const char* forWhat, const char* pw_cmd,
                               unsigned int  max_pass_tries,
                               unsigned int* number_try);
char* getDecryptionPasswordForAccountConfig(const char*   shortname,
                                            const char*   pw_cmd,
                                            unsigned int  max_pass_tries,
                                            unsigned int* number_try);

#endif  // PROMPT_UTILS_H
