#ifndef PROMPT_CRYPT_FILE_UTILS_H
#define PROMPT_CRYPT_FILE_UTILS_H

#include "utils/oidc_error.h"
#include "utils/resultWithEncryptionPassword.h"

oidc_error_t promptEncryptAndWriteToFile(const char* text, const char* filepath,
                                         const char* hint,
                                         const char* suggestedPassword,
                                         const char* pw_cmd,
                                         const char* pw_file,
                                         const char* pw_env);
oidc_error_t promptEncryptAndWriteToOidcFile(
    const char* text, const char* filename, const char* hint,
    const char* suggestedPassword, const char* pw_cmd, const char* pw_file,
    const char* pw_env);
struct resultWithEncryptionPassword getDecryptedFileAndPasswordFor(
    const char* filepath, const char* pw_cmd, const char* pw_file,
    const char* pw_env);
struct resultWithEncryptionPassword getDecryptedOidcFileAndPasswordFor(
    const char* filename, const char* pw_cmd, const char* pw_file,
    const char* pw_env);
char* getDecryptedFileFor(const char* filepath, const char* pw_cmd,
                          const char* pw_file, const char* pw_env);
char* getDecryptedOidcFileFor(const char* filename, const char* pw_cmd,
                              const char* pw_file, const char* pw_env);

#endif  // PROMPT_CRYPT_FILE_UTILS_H
