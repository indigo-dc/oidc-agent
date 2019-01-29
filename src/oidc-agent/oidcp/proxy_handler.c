#include "proxy_handler.h"
#include "oidc-agent/oidc/values.h"
#include "utils/crypt/cryptUtils.h"
#include "utils/json.h"

#include <stdlib.h>

void updateRefreshTokenUsingPassword(const char* shortname,
                                     const char* refresh_token,
                                     const char* password) {
  char*  file_content = decryptOidcFile(shortname, password);
  cJSON* cjson        = stringToJson(file_content);
  secFree(file_content);
  setJSONValue(cjson, OIDC_KEY_REFRESHTOKEN, refresh_token);
  char* updated_content = jsonToString(cjson);
  secFreeJson(cjson);
  encryptAndWriteUsingPassword(updated_content, password, NULL, shortname);
  secFree(updated_content);
}
