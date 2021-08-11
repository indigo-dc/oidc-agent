#include "defines/oidc_values.h"
#include "proxy_handler.h"
#include "utils/crypt/cryptUtils.h"
#include "utils/crypt/gpg/gpg.h"
#include "utils/file_io/cryptFileUtils.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/json.h"

oidc_error_t _updateRT(char* file_content, const char* shortname,
                       const char* refresh_token, const char* password,
                       const char* gpg_key) {
  if (file_content == NULL) {
    return oidc_errno;
  }
  cJSON* cjson = stringToJson(file_content);
  secFree(file_content);
  setJSONValue(cjson, OIDC_KEY_REFRESHTOKEN, refresh_token);
  char* updated_content = jsonToString(cjson);
  secFreeJson(cjson);
  oidc_error_t e =
      encryptAndWriteToOidcFile(updated_content, shortname, password, gpg_key);
  secFree(updated_content);
  return e;
}

oidc_error_t updateRefreshTokenUsingPassword(const char* shortname,
                                             const char* encrypted_content,
                                             const char* refresh_token,
                                             const char* password) {
  if (shortname == NULL || refresh_token == NULL || password == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  char* file_content = decryptFileContent(encrypted_content, password);
  return _updateRT(file_content, shortname, refresh_token, password, NULL);
}

oidc_error_t updateRefreshTokenUsingGPG(const char* shortname,
                                        const char* encrypted_content,
                                        const char* refresh_token,
                                        const char* gpg_key) {
  if (shortname == NULL || refresh_token == NULL || gpg_key == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  char* file_content = decryptPGPFileContent(encrypted_content);
  return _updateRT(file_content, shortname, refresh_token, NULL, gpg_key);
}