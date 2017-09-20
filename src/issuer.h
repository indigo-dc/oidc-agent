#ifndef ISSUER_H
#define ISSUER_H

#include "oidc_utilities.h"

struct oidc_issuer {
  char* issuer_url;
  char* configuration_endpoint;      
  char* token_endpoint;             
  char* authorization_endpoint;
  char* revocation_endpoint;
  char* registration_endpoint;
};

void clearFreeIssuer(struct oidc_issuer* iss) ;
inline static char* issuer_getIssuerUrl(struct oidc_issuer iss) { return iss.issuer_url; };
inline static char* issuer_getConfigEndpoint(struct oidc_issuer iss) { return iss.configuration_endpoint; };
inline static char* issuer_getTokenEndpoint(struct oidc_issuer iss) { return iss.token_endpoint; };
inline static char* issuer_getAuthorizationEndpoint(struct oidc_issuer iss) { return iss.authorization_endpoint; };
inline static char* issuer_getRevocationEndpoint(struct oidc_issuer iss) { return iss.revocation_endpoint; };
inline static char* issuer_getRegistrationEndpoint(struct oidc_issuer iss) { return iss.registration_endpoint; };

inline static void issuer_setIssuerUrl(struct oidc_issuer* iss, char* issuer_url) { clearFreeString(iss->issuer_url); iss->issuer_url=issuer_url; }
inline static void issuer_setConfigurationEndpoint(struct oidc_issuer* iss, char* configuration_endpoint) { clearFreeString(iss->configuration_endpoint); iss->configuration_endpoint=configuration_endpoint; }
inline static void issuer_setTokenEndpoint(struct oidc_issuer* iss, char* token_endpoint) { clearFreeString(iss->token_endpoint); iss->token_endpoint=token_endpoint; }
inline static void issuer_setAuthorizationEndpoint(struct oidc_issuer* iss, char* authorization_endpoint) { clearFreeString(iss->authorization_endpoint); iss->authorization_endpoint=authorization_endpoint; }
inline static void issuer_setRevocationEndpoint(struct oidc_issuer* iss, char* revocation_endpoint) { clearFreeString(iss->revocation_endpoint); iss->revocation_endpoint=revocation_endpoint; }
inline static void issuer_setRegistrationEndpoint(struct oidc_issuer* iss, char* registration_endpoint) { clearFreeString(iss->registration_endpoint); iss->registration_endpoint=registration_endpoint; }

#endif //ISSUER_H
