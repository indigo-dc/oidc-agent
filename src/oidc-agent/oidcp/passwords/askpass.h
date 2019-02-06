#ifndef OIDC_ASKPASS_RUNNER_H
#define OIDC_ASKPASS_RUNNER_H

char* askpass_getPasswordForUpdate(const char* shortname);
char* askpass_getPasswordForAutoload(const char* shortname,
                                     const char* application_hint);
int   askpass_promptConfirmation(const char* prompt_msg);

#endif  // OIDC_ASKPASS_RUNNER_H
