#ifndef OIDC_ASKPASS_RUNNER_H
#define OIDC_ASKPASS_RUNNER_H

#include "utils/oidc_error.h"

char*        askpass_getPasswordForUpdate(const char* shortname);
char*        askpass_getPasswordForAutoload(const char* shortname,
                                            const char* application_hint);
char*        askpass_getPasswordForAutoloadWithIssuer(const char* issuer,
                                                      const char* shortname,
                                                      const char* application_hint);
oidc_error_t askpass_getConfirmation(const char* shortname,
                                     const char* application_hint);
oidc_error_t askpass_getConfirmationWithIssuer(const char* issuer,
                                               const char* shortname,
                                               const char* application_hint);
oidc_error_t askpass_getIdTokenConfirmation(const char* shortname,
                                            const char* application_hint);
oidc_error_t askpass_getIdTokenConfirmationWithIssuer(
    const char* issuer, const char* shortname, const char* application_hint);
char* askpass_getMytokenConfirmation(const char* base64_html);

#endif  // OIDC_ASKPASS_RUNNER_H
