#include "proxy_handler.h"

#include <string.h>

#include "account/issuer_helper.h"
#include "defines/settings.h"
#include "oidc-agent/oidcp/passwords/askpass.h"
#include "oidc-agent/oidcp/passwords/password_store.h"
#include "utils/crypt/cryptUtils.h"
#include "utils/crypt/gpg/gpg.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/listUtils.h"
#include "utils/string/stringUtils.h"

oidc_error_t updateRefreshToken(const char* shortname,
                                const char* refresh_token) {
  if (shortname == NULL || refresh_token == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  char* encrypted_content = readOidcFile(shortname);
  if (!isPGPMessage(encrypted_content)) {
    char*        password = getPasswordFor(shortname);
    oidc_error_t e        = updateRefreshTokenUsingPassword(
               shortname, encrypted_content, refresh_token, password);
    secFree(encrypted_content);
    secFree(password);
    return e;
  } else {
    char* gpg_key = getGPGKeyFor(shortname);
    if (gpg_key == NULL) {
      gpg_key = extractPGPKeyID(encrypted_content);
    }
    oidc_error_t e = updateRefreshTokenUsingGPG(shortname, encrypted_content,
                                                refresh_token, gpg_key);
    secFree(encrypted_content);
    secFree(gpg_key);
    return e;
  }
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
  char* crypt_content = readOidcFile(shortname);
  if (crypt_content == NULL) {
    return NULL;
  }
  if (isPGPMessage(crypt_content)) {
    char* config = decryptPGPFileContent(crypt_content);
    secFree(crypt_content);
    return config;
  }
  for (size_t i = 0; i < MAX_PASS_TRIES; i++) {
    char* password =
        issuer ? askpass_getPasswordForAutoloadWithIssuer(issuer, shortname,
                                                          application_hint)
               : askpass_getPasswordForAutoload(shortname, application_hint);
    if (password == NULL) {
      secFree(crypt_content);
      return NULL;
    }
    char* config = decryptFileContent(crypt_content, password);
    secFree(password);
    if (config != NULL) {
      secFree(crypt_content);
      return config;
    }
  }
  secFree(crypt_content);
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
