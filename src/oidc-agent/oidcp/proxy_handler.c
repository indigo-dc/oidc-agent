#include "proxy_handler.h"

#include <string.h>

#include "account/issuer_helper.h"
#include "defines/settings.h"
#include "oidc-agent/oidcp/passwords/askpass.h"
#include "oidc-agent/oidcp/passwords/password_store.h"
#include "utils/config/issuerConfig.h"
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
    oidc_error_t e = OIDC_EERROR;
    for (int i = 0; i < MAX_PASS_TRIES && e != OIDC_SUCCESS; i++) {
      char* password = getPasswordFor(shortname);
      if (password == NULL) {
        e = OIDC_EUSRPWCNCL;
        break;
      }
      e = updateRefreshTokenUsingPassword(shortname, encrypted_content,
                                          refresh_token, password);
      secFree(password);
    }
    secFree(encrypted_content);
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

const char* getDefaultAccountConfigForIssuer(const char* issuer_url) {
  if (issuer_url == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  const struct issuerConfig* c = getIssuerConfig(issuer_url);
  if (c == NULL) {
    return NULL;
  }
  if (strValid(c->default_account)) {
    return c->default_account;
  }
  if (!listValid(c->accounts)) {
    return NULL;
  }
  list_node_t* firstAccount = list_at(c->accounts, 0);
  return firstAccount ? firstAccount->val : NULL;
}
