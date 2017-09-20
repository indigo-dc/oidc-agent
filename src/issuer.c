#include "issuer.h"
#include "oidc_utilities.h"

void clearFreeIssuer(struct oidc_issuer* iss) {
  if(!iss) {
    return;
  }
  clearFreeString(iss->issuer_url);
  clearFreeString(iss->configuration_endpoint);
  clearFreeString(iss->token_endpoint);
  clearFreeString(iss->authorization_endpoint);
  clearFreeString(iss->revocation_endpoint);
  clearFreeString(iss->registration_endpoint);
  clearFree(iss, sizeof(struct oidc_issuer));
}

