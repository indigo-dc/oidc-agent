#include "error.h"

#include "utils/oidc_error.h"

char* oidcagent_serror() { return oidc_serror(); }

void oidcagent_perror() { oidc_perror(); }
