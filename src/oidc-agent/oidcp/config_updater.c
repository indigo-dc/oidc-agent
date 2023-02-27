#include "config_updater.h"

#include "defines/oidc_values.h"
#include "defines/settings.h"
#include "oidc-agent/oidcp/passwords/password_store.h"
#include "proxy_handler.h"
#include "utils/config/gen_config.h"
#include "utils/crypt/cryptUtils.h"
#include "utils/crypt/gpg/gpg.h"
#include "utils/file_io/cryptFileUtils.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/json.h"
#include "utils/promptUtils.h"
#include "utils/string/stringUtils.h"

oidc_error_t _updateRT(char* file_content, const char* shortname,
                       const char* refresh_token, const char* password,
                       const char* gpg_key) {
  if (file_content == NULL) {
    oidc_setArgNullFuncError(__func__);
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
  if (file_content == NULL) {
    return oidc_errno;
  }
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
  if (file_content == NULL) {
    return oidc_errno;
  }
  return _updateRT(file_content, shortname, refresh_token, NULL, gpg_key);
}

oidc_error_t writeOIDCFile(const char* content, const char* shortname) {
  if (content == NULL || shortname == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  char* gpg_key =
      getGPGKeyFor(shortname) ?: extractPGPKeyIDFromOIDCFile(shortname);
  if (gpg_key == NULL && !oidcFileDoesExist(shortname) &&
      getGenConfig()->default_gpg_key) {
    gpg_key = oidc_strcopy(getGenConfig()->default_gpg_key);
  }
  char* password = NULL;
  if (gpg_key == NULL) {
    if (!oidcFileDoesExist(shortname)) {
      password = getEncryptionPasswordForAccountConfig(shortname, NULL, NULL,
                                                       NULL, NULL);
    } else {
      char* file_content = NULL;
      for (int i = 0; i < MAX_PASS_TRIES && file_content == NULL; i++) {
        secFree(password);
        password = getPasswordFor(shortname);
        if (password == NULL) {
          oidc_errno = OIDC_EUSRPWCNCL;
          return oidc_errno;
        }
        file_content = decryptOidcFile(shortname, password);
      }
      if (file_content == NULL) {
        secFree(password);
        return oidc_errno;
      }
      secFree(file_content);
    }
  }
  oidc_error_t ret =
      encryptAndWriteToOidcFile(content, shortname, password, gpg_key);
  secFree(gpg_key);
  secFree(password);
  return ret;
}