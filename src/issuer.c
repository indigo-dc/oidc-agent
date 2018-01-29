#include "issuer.h"
#include "json.h"
#include "oidc_error.h"
#include "oidc_utilities.h"

#include <syslog.h>

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

char* getUsableGrantTypes(const char* supported, int usePasswordGrantType) {
  if(supported==NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  list_t* supp = JSONArrayToList(supported);
  list_t* wanted = delimitedStringToList(usePasswordGrantType ? "refresh_token authorization_code password" : "refresh_token authorization_code", ' ');
  list_t* usable = intersectLists(wanted, supp);
  list_destroy(supp);
  list_destroy(wanted);
  char* str = listToJSONArray(usable);
  list_destroy(usable);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "usable grant types are: %s", str);
  return str;
}

