#ifndef GENERAL_HANDLER_UTILS_H
#define GENERAL_HANDLER_UTILS_H

// #ifdef INCLUDE_WRITE_HANDLERS

#include "utils/oidc_error.h"

oidc_error_t encryptAndWriteText(const char* text, const char* hint,
                                 const char* suggestedPassword,
                                 const char* filepath,
                                 const char* oidc_filename);
char* getEncryptionPassword(const char* forWhat, const char* suggestedPassword,
                            unsigned int max_pass_tries);
oidc_error_t encryptAndWriteWithPassword(const char* text, const char* password,
                                         const char* filepath,
                                         const char* oidc_filename);

// #endif  // INCLUDE_WRITE_HANDLERS

#endif  // GENERAL_HANDLER_UTILS_H
