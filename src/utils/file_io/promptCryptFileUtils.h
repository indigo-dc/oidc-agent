#ifndef PROMPT_CRYPT_FILE_UTILS_H
#define PROMPT_CRYPT_FILE_UTILS_H

#include "utils/oidc_error.h"

struct resultWithEncryptionPassword {
  void* result;
  char* password;
};

#define RESULT_WITH_PASSWORD_NULL \
  (struct resultWithEncryptionPassword) { .result = NULL, .password = NULL }

oidc_error_t promptEncryptAndWriteToFile(const char* text, const char* filepath,
                                         const char* hint,
                                         const char* suggestedPassword,
                                         const char* pw_cmd);
oidc_error_t promptEncryptAndWriteToOidcFile(const char* text,
                                             const char* filename,
                                             const char* hint,
                                             const char* suggestedPassword,
                                             const char* pw_cmd);
struct resultWithEncryptionPassword getDecryptedFileAndPasswordFor(
    const char* filepath, const char* pw_cmd);
struct resultWithEncryptionPassword getDecryptedOidcFileAndPasswordFor(
    const char* filename, const char* pw_cmd);
char* getDecryptedFileFor(const char* filepath, const char* pw_cmd);
char* getDecryptedOidcFileFor(const char* filename, const char* pw_cmd);

#endif  // PROMPT_CRYPT_FILE_UTILS_H
