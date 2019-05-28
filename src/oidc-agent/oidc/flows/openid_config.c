#include "openid_config.h"

#include "account/account.h"
#include "defines/settings.h"
#include "oidc-agent/http/http_ipc.h"
#include "oidc-agent/oidc/parse_oidp.h"
#include "utils/logger.h"
#include "utils/oidc_error.h"

/** @fn oidc_error_t getIssuerConfig(struct oidc_account* account)
 * @brief retrieves issuer config from the configuration_endpoint
 * @note the issuer url has to be set prior
 * @param account the account struct, will be updated with the retrieved
 * config
 * @return a oidc_error status code
 */
oidc_error_t getIssuerConfig(struct oidc_account* account) {
  char* configuration_endpoint =
      oidc_strcat(account_getIssuerUrl(account), CONF_ENDPOINT_SUFFIX);
  issuer_setConfigurationEndpoint(account_getIssuer(account),
                                  configuration_endpoint);
  logger(DEBUG, "Configuration endpoint is: %s",
         account_getConfigEndpoint(account));
  char* res = httpsGET(account_getConfigEndpoint(account), NULL,
                       account_getCertPath(account));
  if (NULL == res) {
    return oidc_errno;
  }
  return parseOpenidConfiguration(res, account);
}
