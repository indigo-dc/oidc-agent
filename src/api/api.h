#ifndef OIDC_API_H
#define OIDC_API_H

#include "api/export_symbols.h"


char* communicate(unsigned char remote, const char* fmt, ...);
char* oidcagent_serror();
void oidcagent_perror();

/**
 * @brief gets an error string detailing the last occurred error
 * @return the error string. MUST NOT be freed.
 */
LIB_PUBLIC char* oidcagent_serror();

/**
 * @brief prints an error message to stderr detailing the last occurred error
 */
LIB_PUBLIC void oidcagent_perror();

extern LIB_PUBLIC void _secFree(void*);

#ifndef secFree
#define secFree(ptr) \
  do {               \
    _secFree((ptr)); \
    (ptr) = NULL;    \
  } while (0)
#endif  // secFree

#endif  // OIDC_API_H
