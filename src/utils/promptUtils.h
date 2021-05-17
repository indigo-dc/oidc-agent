#ifndef PROMPT_UTILS_H
#define PROMPT_UTILS_H

#include "utils/resultWithEncryptionPassword.h"

char* getEncryptionPasswordFor(const char* forWhat,
                               const char* suggestedPassword,
                               const char* pw_cmd, const char* pw_file,
                               const char* pw_env);
char* getEncryptionPasswordForAccountConfig(const char* shortname,
                                            const char* suggestedPassword,
                                            const char* pw_cmd,
                                            const char* pw_file,
                                            const char* pw_env);
char* getDecryptionPasswordFor(const char* forWhat, const char* pw_cmd,
                               const char* pw_file, const char* pw_env,
                               unsigned int  max_pass_tries,
                               unsigned int* number_try);
char* getDecryptionPasswordForAccountConfig(
    const char* shortname, const char* pw_cmd, const char* pw_file,
    const char* pw_env, unsigned int max_pass_tries, unsigned int* number_try);

struct resultWithEncryptionPassword _getDecryptedTextAndPasswordWithPromptFor(
    const char* decrypt_argument, const char*                prompt_argument,
    char* (*const decryptFnc)(const char*, const char*), int isAccountConfig,
    const char* pw_cmd, const char* pw_file, const char* pw_env);
char* getDecryptedTextWithPromptFor(
    const char* decrypt_argument, const char*                prompt_argument,
    char* (*const decryptFnc)(const char*, const char*), int isAccountConfig,
    const char* pw_cmd, const char* pw_file, const char* pw_env);

#endif  // PROMPT_UTILS_H
