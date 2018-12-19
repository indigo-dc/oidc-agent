#ifndef ISSUER_H
#define ISSUER_H

#include "utils/memory.h"

struct device_authorization_endpoint {
  char* url;
  int   setByUser;
};

struct oidc_issuer {
  char* issuer_url;

  char*                                configuration_endpoint;
  char*                                token_endpoint;
  char*                                authorization_endpoint;
  char*                                revocation_endpoint;
  char*                                registration_endpoint;
  struct device_authorization_endpoint device_authorization_endpoint;

  char* scopes_supported;          // space delimited
  char* grant_types_supported;     // as json array
  char* response_types_supported;  // as json array
};

void                _secFreeIssuer(struct oidc_issuer* iss);
inline static char* issuer_getIssuerUrl(struct oidc_issuer* iss) {
  return iss ? iss->issuer_url : NULL;
};
inline static char* issuer_getConfigEndpoint(struct oidc_issuer* iss) {
  return iss ? iss->configuration_endpoint : NULL;
};
inline static char* issuer_getTokenEndpoint(struct oidc_issuer* iss) {
  return iss ? iss->token_endpoint : NULL;
};
inline static char* issuer_getAuthorizationEndpoint(struct oidc_issuer* iss) {
  return iss ? iss->authorization_endpoint : NULL;
};
inline static char* issuer_getRevocationEndpoint(struct oidc_issuer* iss) {
  return iss ? iss->revocation_endpoint : NULL;
};
inline static char* issuer_getRegistrationEndpoint(struct oidc_issuer* iss) {
  return iss ? iss->registration_endpoint : NULL;
};
inline static char* issuer_getDeviceAuthorizationEndpoint(
    struct oidc_issuer* iss) {
  return iss ? iss->device_authorization_endpoint.url : NULL;
};
inline static int issuer_getDeviceAuthorizationEndpointIsSetByUser(
    struct oidc_issuer* iss) {
  return iss ? iss->device_authorization_endpoint.setByUser : 0;
}
inline static char* issuer_getScopesSupported(struct oidc_issuer* iss) {
  return iss ? iss->scopes_supported : NULL;
}
inline static char* issuer_getResponseTypesSupported(struct oidc_issuer* iss) {
  return iss ? iss->response_types_supported : NULL;
}
inline static char* issuer_getGrantTypesSupported(struct oidc_issuer* iss) {
  return iss ? iss->grant_types_supported : NULL;
}

inline static void issuer_setIssuerUrl(struct oidc_issuer* iss,
                                       char*               issuer_url) {
  if (iss->issuer_url == issuer_url) {
    return;
  }
  secFree(iss->issuer_url);
  iss->issuer_url = issuer_url;
}
inline static void issuer_setConfigurationEndpoint(
    struct oidc_issuer* iss, char* configuration_endpoint) {
  if (iss->configuration_endpoint == configuration_endpoint) {
    return;
  }
  secFree(iss->configuration_endpoint);
  iss->configuration_endpoint = configuration_endpoint;
}
inline static void issuer_setTokenEndpoint(struct oidc_issuer* iss,
                                           char*               token_endpoint) {
  if (iss->token_endpoint == token_endpoint) {
    return;
  }
  secFree(iss->token_endpoint);
  iss->token_endpoint = token_endpoint;
}
inline static void issuer_setAuthorizationEndpoint(
    struct oidc_issuer* iss, char* authorization_endpoint) {
  if (iss->authorization_endpoint == authorization_endpoint) {
    return;
  }
  secFree(iss->authorization_endpoint);
  iss->authorization_endpoint = authorization_endpoint;
}
inline static void issuer_setRevocationEndpoint(struct oidc_issuer* iss,
                                                char* revocation_endpoint) {
  if (iss->revocation_endpoint == revocation_endpoint) {
    return;
  }
  secFree(iss->revocation_endpoint);
  iss->revocation_endpoint = revocation_endpoint;
}
inline static void issuer_setRegistrationEndpoint(struct oidc_issuer* iss,
                                                  char* registration_endpoint) {
  if (iss->registration_endpoint == registration_endpoint) {
    return;
  }
  secFree(iss->registration_endpoint);
  iss->registration_endpoint = registration_endpoint;
}
inline static void issuer_setDeviceAuthorizationEndpoint(
    struct oidc_issuer* iss, char* device_authorization_endpoint,
    int setByUser) {
  if (iss->device_authorization_endpoint.url == device_authorization_endpoint) {
    return;
  }
  secFree(iss->device_authorization_endpoint.url);
  iss->device_authorization_endpoint.url       = device_authorization_endpoint;
  iss->device_authorization_endpoint.setByUser = setByUser;
}
inline static void issuer_setScopesSupported(struct oidc_issuer* iss,
                                             char* scopes_supported) {
  if (iss->scopes_supported == scopes_supported) {
    return;
  }
  secFree(iss->scopes_supported);
  iss->scopes_supported = scopes_supported;
}
inline static void issuer_setGrantTypesSupported(struct oidc_issuer* iss,
                                                 char* grant_types_supported) {
  if (iss->grant_types_supported == grant_types_supported) {
    return;
  }
  secFree(iss->grant_types_supported);
  iss->grant_types_supported = grant_types_supported;
}
inline static void issuer_setResponseTypesSupported(
    struct oidc_issuer* iss, char* response_types_supported) {
  if (iss->response_types_supported == response_types_supported) {
    return;
  }
  secFree(iss->response_types_supported);
  iss->response_types_supported = response_types_supported;
}

#ifndef secFreeIssuer
#define secFreeIssuer(ptr) \
  do {                     \
    _secFreeIssuer((ptr)); \
    (ptr) = NULL;          \
  } while (0)
#endif  // secFreeIssuer

#endif  // ISSUER_H
