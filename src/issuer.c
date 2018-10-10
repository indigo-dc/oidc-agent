#include "issuer.h"

void secFreeIssuer(struct oidc_issuer* iss) {
  if (!iss) {
    return;
  }
  issuer_setIssuerUrl(iss, NULL);
  issuer_setConfigurationEndpoint(iss, NULL);
  issuer_setTokenEndpoint(iss, NULL);
  issuer_setAuthorizationEndpoint(iss, NULL);
  issuer_setRevocationEndpoint(iss, NULL);
  issuer_setRegistrationEndpoint(iss, NULL);
  issuer_setDeviceAuthorizationEndpoint(iss, NULL);
  issuer_setScopesSupported(iss, NULL);
  issuer_setGrantTypesSupported(iss, NULL);
  issuer_setResponseTypesSupported(iss, NULL);
  secFree(iss);
  iss = NULL;
}
