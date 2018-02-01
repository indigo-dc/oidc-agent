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

  char* scopes_supported; // space delimited
  char* grant_types_supported; // as json array
  char* response_types_supported; // as json array
};

void clearFreeIssuer(struct oidc_issuer* iss) ;
inline static char* issuer_getIssuerUrl(struct oidc_issuer iss) { return iss.issuer_url; };
inline static char* issuer_getConfigEndpoint(struct oidc_issuer iss) { return iss.configuration_endpoint; };
inline static char* issuer_getTokenEndpoint(struct oidc_issuer iss) { return iss.token_endpoint; };
inline static char* issuer_getAuthorizationEndpoint(struct oidc_issuer iss) { return iss.authorization_endpoint; };
inline static char* issuer_getRevocationEndpoint(struct oidc_issuer iss) { return iss.revocation_endpoint; };
inline static char* issuer_getRegistrationEndpoint(struct oidc_issuer iss) { return iss.registration_endpoint; };
inline static char* issuer_getScopesSupported(struct oidc_issuer iss) { return iss.scopes_supported; }
inline static char* issuer_getResponseTypesSupported(struct oidc_issuer iss) { return iss.response_types_supported; }
inline static char* issuer_getGrantTypesSupported(struct oidc_issuer iss) { return iss.grant_types_supported; }

inline static void issuer_setIssuerUrl(struct oidc_issuer* iss, char* issuer_url) { clearFreeString(iss->issuer_url); iss->issuer_url=issuer_url; }
inline static void issuer_setConfigurationEndpoint(struct oidc_issuer* iss, char* configuration_endpoint) { clearFreeString(iss->configuration_endpoint); iss->configuration_endpoint=configuration_endpoint; }
inline static void issuer_setTokenEndpoint(struct oidc_issuer* iss, char* token_endpoint) { clearFreeString(iss->token_endpoint); iss->token_endpoint=token_endpoint; }
inline static void issuer_setAuthorizationEndpoint(struct oidc_issuer* iss, char* authorization_endpoint) { clearFreeString(iss->authorization_endpoint); iss->authorization_endpoint=authorization_endpoint; }
inline static void issuer_setRevocationEndpoint(struct oidc_issuer* iss, char* revocation_endpoint) { clearFreeString(iss->revocation_endpoint); iss->revocation_endpoint=revocation_endpoint; }
inline static void issuer_setRegistrationEndpoint(struct oidc_issuer* iss, char* registration_endpoint) { clearFreeString(iss->registration_endpoint); iss->registration_endpoint=registration_endpoint; }
inline static void issuer_setScopesSupported(struct oidc_issuer* iss, char* scopes_supported) { clearFreeString(iss->scopes_supported); iss->scopes_supported = scopes_supported; }
inline static void issuer_setGrantTypesSupported(struct oidc_issuer* iss, char* grant_types_supported) { clearFreeString(iss->grant_types_supported); iss->grant_types_supported = grant_types_supported; }
inline static void issuer_setResponseTypesSupported(struct oidc_issuer* iss, char* response_types_supported) { clearFreeString(iss->response_types_supported); iss->response_types_supported = response_types_supported; }

#endif //ISSUER_H
