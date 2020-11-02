#ifndef OIDC_GEN_PROMPT_CONSENT_H
#define OIDC_GEN_PROMPT_CONSENT_H

#include "oidc-gen/oidc-gen_options.h"

int gen_promptConsentDefaultNo(const char*             text,
                               const struct arguments* arguments);
int gen_promptConsentDefaultYes(const char*             text,
                                const struct arguments* arguments);

#endif  // OIDC_GEN_PROMPT_CONSENT_H
