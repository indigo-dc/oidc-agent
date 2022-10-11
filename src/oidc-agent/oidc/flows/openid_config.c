#include "openid_config.h"

#include <string.h>

#include "account/account.h"
#include "defines/mytoken_values.h"
#include "defines/settings.h"
#include "oidc-agent/http/http_ipc.h"
#include "oidc-agent/oidc/parse_oidp.h"
#include "utils/agentLogger.h"
#include "utils/json.h"
#include "utils/oidc_error.h"
#include "utils/string/stringUtils.h"

char* _obtainIssuerConfig(struct oidc_account* account) {
  char* cert_path = account_getCertPath(account);
  char* res       = NULL;
  if (strValid(account_getConfigEndpoint(account))) {
    res = httpsGET(account_getConfigEndpoint(account), NULL, cert_path);
  } else {
    const char* iss_url =
        account_getMytokenUrl(account) ?: account_getIssuerUrl(account);
    char* openid_configuration_endpoint =
        oidc_sprintf("%s%s%s", iss_url, lastChar(iss_url) == '/' ? "" : "/",
                     CONF_ENDPOINT_SUFFIX);
    char* oauth2_configuration_endpoint =
        oidc_sprintf("%s%s%s", iss_url, lastChar(iss_url) == '/' ? "" : "/",
                     OAUTH_CONF_ENDPOINT_SUFFIX);
    char* configuration_endpoint = account_getIsOAuth2(account)
                                       ? oauth2_configuration_endpoint
                                       : openid_configuration_endpoint;
    char* second_try             = account_getIsOAuth2(account)
                                       ? openid_configuration_endpoint
                                       : oauth2_configuration_endpoint;
    res = httpsGET(configuration_endpoint, NULL, cert_path);
    if (res == NULL && oidc_errno == 404) {
      account_setOAuth2(account);  // either it was already set or
                                   // openid-configuration was not found
      secFree(configuration_endpoint);
      configuration_endpoint = second_try;
      res = httpsGET(configuration_endpoint, NULL, cert_path);
    } else {
      secFree(second_try);
    }
    account_setConfigEndpoint(account, configuration_endpoint);
  }
  agent_log(DEBUG, "Configuration endpoint is: %s",
            account_getConfigEndpoint(account));
  return res;
}
/** @fn oidc_error_t obtainIssuerConfig(struct oidc_account* account)
 * @brief retrieves issuer config from the configuration_endpoint
 * @note the issuer url has to be set prior
 * @param account the account struct, will be updated with the retrieved
 * config
 * @return a oidc_error status code
 */
oidc_error_t obtainIssuerConfig(struct oidc_account* account) {
  char* res = _obtainIssuerConfig(account);
  if (NULL == res) {
    return oidc_errno;
  }
  return parseOpenidConfiguration(res, account);
}

char* getScopesSupportedFor(const char* issuer_url, const char* config_endpoint,
                            const char* cert_path) {
  struct oidc_account account = {};
  account_setIssuerUrl(&account, oidc_strcopy(issuer_url));
  account_setConfigEndpoint(&account, oidc_strcopy(config_endpoint));
  if (strValid(cert_path)) {
    account_setCertPath(&account, oidc_strcopy(cert_path));
  } else {
    account_setOSDefaultCertPath(&account);
  }
  oidc_error_t err = obtainIssuerConfig(&account);
  if (err != OIDC_SUCCESS) {
    return NULL;
  }
  char* scopes = oidc_strcopy(account_getScopesSupported(&account));
  secFreeAccountContent(&account);
  return scopes;
}

char* getProvidersSupportedByMytoken(const char* issuer_url,
                                     const char* config_endpoint,
                                     const char* cert_path) {
  struct oidc_account account = {};
  account_setIssuerUrl(&account, oidc_strcopy(issuer_url));
  account_setConfigEndpoint(&account, oidc_strcopy(config_endpoint));
  if (strValid(cert_path)) {
    account_setCertPath(&account, oidc_strcopy(cert_path));
  } else {
    account_setOSDefaultCertPath(&account);
  }
  char* res = _obtainIssuerConfig(&account);
  if (res == NULL) {
    return NULL;
  }
  char* providers =
      getJSONValueFromString(res, MYTOKEN_KEY_PROVIDERS_SUPPORTED);
  secFree(res);
  secFreeAccountContent(&account);
  return providers;
}
