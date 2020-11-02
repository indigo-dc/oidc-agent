#include "proxy_handler.h"

#include "defines/oidc_values.h"
#include "utils/file_io/cryptFileUtils.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/json.h"

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
      encryptAndWriteToOidcFile(updated_content, shortname, password);
  secFree(updated_content);
  return e;
}
