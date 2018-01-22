#include "issuer.h"
#include "oidc_utilities.h"

void clearFreeIssuer(struct oidc_issuer* iss) {
  if(!iss) {
    return;
  }
  issuer_setIssuerUrl(iss, NULL);
  issuer_setConfigurationEndpoint(iss, NULL);
  issuer_setTokenEndpoint(iss, NULL);
  issuer_setAuthorizationEndpoint(iss, NULL);
  issuer_setRevocationEndpoint(iss, NULL);
  issuer_setRegistrationEndpoint(iss, NULL);
  issuer_setScopesSupported(iss, NULL);
  issuer_setGrantTypesSupported(iss, NULL);
  issuer_setResponseTypesSupported(iss, NULL);
  clearFree(iss, sizeof(struct oidc_issuer));
  iss = NULL;
}

