#ifndef OIDC_AGENT_ERROR_H
#define OIDC_AGENT_ERROR_H

#include "export_symbols.h"

/**
 * @brief gets an error string detailing the last occurred error
 * @return the error string. MUST NOT be freed.
 */
LIB_PUBLIC char* oidcagent_serror();

/**
 * @brief prints an error message to stderr detailing the last occurred error
 */
LIB_PUBLIC void oidcagent_perror();

#endif //OIDC_AGENT_ERROR_H
