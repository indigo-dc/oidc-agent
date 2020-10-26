#include "proxy_handler.h"
#include "account/issuer_helper.h"
#include "defines/settings.h"
#include "oidc-agent/oidcp/passwords/askpass.h"
#include "oidc-agent/oidcp/passwords/password_store.h"
#include "utils/crypt/cryptUtils.h"
#include "utils/file_io/cryptFileUtils.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/listUtils.h"
#include "utils/stringUtils.h"

#include <stdlib.h>
#include <string.h>

oidc_error_t updateRefreshToken(const char* shortname,
                                const char* refresh_token) {
  if (shortname == NULL || refresh_token == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  char*        password = getPasswordFor(shortname);
  oidc_error_t e =
      updateRefreshTokenUsingPassword(shortname, refresh_token, password);
  secFree(password);
  return e;
}

char* getAutoloadConfig(const char* shortname, const char* issuer,
                        const char* application_hint) {
  if (shortname == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  if (!oidcFileDoesExist(shortname)) {
    oidc_errno = OIDC_ENOACCOUNT;
    return NULL;
  }
  for (size_t i = 0; i < MAX_PASS_TRIES; i++) {
    char* password =
        issuer ? askpass_getPasswordForAutoloadWithIssuer(issuer, shortname,
                                                          application_hint)
               : askpass_getPasswordForAutoload(shortname, application_hint);
    if (password == NULL) {
      return NULL;
    }
    char* config = decryptOidcFile(shortname, password);
    secFree(password);
    if (config != NULL) {
      return config;
    }
  }
  return NULL;
}

char* getDefaultAccountConfigForIssuer(const char* issuer_url) {
  if (issuer_url == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  list_t* issuers = getLinesFromOidcFile(ISSUER_CONFIG_FILENAME);
  if (issuers == NULL) {
    return NULL;
  }
  char*            shortname = NULL;
  list_node_t*     node;
  list_iterator_t* it = list_iterator_new(issuers, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    char* line = node->val;
    char* iss  = strtok(line, " ");
    char* acc  = strtok(NULL, " ");
    if (compIssuerUrls(issuer_url, iss)) {
      if (strValid(acc)) {
        shortname = oidc_strcopy(acc);
      }
      break;
    }
  }
  list_iterator_destroy(it);
  secFreeList(issuers);
  return shortname;
}
