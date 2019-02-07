#include "proxy_handler.h"
#include "defines/oidc_values.h"
#include "oidc-agent/oidcp/passwords/askpass.h"
#include "oidc-agent/oidcp/passwords/password_store.h"
#include "utils/crypt/cryptUtils.h"
#include "utils/json.h"

#include <stdlib.h>

oidc_error_t updateRefreshTokenUsingPassword(const char* shortname,
                                             const char* refresh_token,
                                             const char* password) {
  if (shortname == NULL || password == NULL || refresh_token == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  char* file_content = decryptOidcFile(shortname, password);
  if (file_content == NULL) {
    return oidc_errno;
  }
  cJSON* cjson = stringToJson(file_content);
  secFree(file_content);
  setJSONValue(cjson, OIDC_KEY_REFRESHTOKEN, refresh_token);
  char* updated_content = jsonToString(cjson);
  secFreeJson(cjson);
  oidc_error_t e =
      encryptAndWriteUsingPassword(updated_content, password, NULL, shortname);
  secFree(updated_content);
  return e;
}

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

char* getAutoloadConfig(const char* shortname, const char* application_hint) {
  if (shortname == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  char* password = askpass_getPasswordForAutoload(shortname, application_hint);
  if (password == NULL) {
    return NULL;
  }
  char* config = decryptOidcFile(shortname, password);
  secFree(password);
  return config;
}
