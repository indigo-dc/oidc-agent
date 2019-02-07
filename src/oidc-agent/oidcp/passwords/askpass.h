#ifndef OIDC_ASKPASS_RUNNER_H
#define OIDC_ASKPASS_RUNNER_H

#include "utils/oidc_error.h"

char*        askpass_getPasswordForUpdate(const char* shortname);
char*        askpass_getPasswordForAutoload(const char* shortname,
                                            const char* application_hint);
oidc_error_t askpass_promptConfirmation(const char* prompt_msg);
oidc_error_t askpass_getConfirmation(const char* shortname,
                                     const char* application_hint);

#endif  // OIDC_ASKPASS_RUNNER_H
