#include "proxy_handler.h"
#include "oidc-agent/oidc/values.h"
#include "oidc-agent/oidcp/password_handler.h"
#include "utils/crypt/cryptUtils.h"
#include "utils/json.h"

#include <stdlib.h>

oidc_error_t updateRefreshTokenUsingPassword(const char* shortname,
                                             const char* refresh_token,
                                             const char* password) {
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
  return OIDC_SUCCESS;
  char*        password = getPasswordFor(shortname);
  oidc_error_t e =
      updateRefreshTokenUsingPassword(shortname, refresh_token, password);
  secFree(password);
  return e;
}
