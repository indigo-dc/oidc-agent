#include "parse_oidp.h"

#include "account/account.h"
#include "account/issuer_helper.h"
#include "defines/oidc_values.h"
#include "device_code.h"
#include "utils/agentLogger.h"
#include "utils/json.h"
#include "utils/key_value.h"
#include "utils/parseJson.h"
#include "utils/string/stringUtils.h"

struct oidc_device_code* parseDeviceCode(const char* res) {
  if (!isJSONObject(res)) {
    return NULL;
  }
  char* error = parseForError(oidc_strcopy(res));
  if (error) {
    oidc_seterror(error);
    oidc_errno = OIDC_EOIDC;
    secFree(error);
    return NULL;
  }
  return getDeviceCodeFromJSON(res);
}

oidc_error_t parseOpenidConfiguration(char* res, struct oidc_account* account) {
  INIT_KEY_VALUE(
      OIDC_KEY_ISSUER, OIDC_KEY_TOKEN_ENDPOINT, OIDC_KEY_AUTHORIZATION_ENDPOINT,
      OIDC_KEY_REGISTRATION_ENDPOINT, OIDC_KEY_REVOCATION_ENDPOINT,
      OIDC_KEY_DEVICE_AUTHORIZATION_ENDPOINT, OIDC_KEY_SCOPES_SUPPORTED,
      OIDC_KEY_GRANT_TYPES_SUPPORTED, OIDC_KEY_RESPONSE_TYPES_SUPPORTED,
      OIDC_KEY_CODE_CHALLENGE_METHODS_SUPPORTED);
  if (CALL_GETJSONVALUES(res) < 0) {
    secFree(res);
    if (oidc_errno == OIDC_EJSONPARS) {
      oidc_errno = OIDC_EOPNOJSON;
    }
    return oidc_errno;
  }
  secFree(res);
  KEY_VALUE_VARS(issuer, token_endpoint, authorization_endpoint,
                 registration_endpoint, revocation_endpoint,
                 device_authorization_endpoint, scopes_supported,
                 grant_types_supported, response_types_supported,
                 code_challenge_method_supported);
  if (_token_endpoint == NULL) {
    agent_log(ERROR, "Could not get token endpoint");
    SEC_FREE_KEY_VALUES();
    oidc_seterror(
        "Could not get token endpoint from the configuration endpoint. This "
        "could be because of a network issue or because the issuer is not "
        "correct.");
    oidc_errno = OIDC_EERROR;
    return oidc_errno;
  }
  struct oidc_issuer* issuer = account_getIssuer(account);
  if (strValid(account_getIssuerUrl(account)) &&
      !compIssuerUrls(_issuer, account_getIssuerUrl(account))) {
    agent_log(ERROR,
              "Issuer url from configuration endpoint ('%s') does not match "
              "expected issuer ('%s')",
              _issuer, account_getIssuerUrl(account));
    SEC_FREE_KEY_VALUES();
    oidc_seterror(
        "Provider uses another issuer url than expected. Something is fishy.");
    oidc_errno = OIDC_EERROR;
    return oidc_errno;
  }
  issuer_setIssuerUrl(issuer, _issuer);
  if (_token_endpoint) {
    issuer_setTokenEndpoint(issuer, _token_endpoint);
  }
  if (_authorization_endpoint) {
    issuer_setAuthorizationEndpoint(issuer, _authorization_endpoint);
  }
  if (_registration_endpoint) {
    issuer_setRegistrationEndpoint(issuer, _registration_endpoint);
  }
  if (_revocation_endpoint) {
    issuer_setRevocationEndpoint(issuer, _revocation_endpoint);
  }
  if (_device_authorization_endpoint) {
    issuer_setDeviceAuthorizationEndpoint(issuer,
                                          _device_authorization_endpoint, 0);
  }
  if (_grant_types_supported == NULL) {
    const char* defaultValue = OIDC_PROVIDER_DEFAULT_GRANTTYPES;
    _grant_types_supported   = oidc_sprintf("%s", defaultValue);
  }
  char* scopes_supported =
      JSONArrayStringToDelimitedString(_scopes_supported, " ");
  if (scopes_supported == NULL) {
    secFree(_scopes_supported);
    secFree(_grant_types_supported);
    secFree(_response_types_supported);
    secFree(_code_challenge_method_supported);
    return oidc_errno;
  }
  account_setScopesSupported(account, scopes_supported);
  secFree(_scopes_supported);
  issuer_setGrantTypesSupported(account_getIssuer(account),
                                _grant_types_supported);
  issuer_setResponseTypesSupported(account_getIssuer(account),
                                   _response_types_supported);
  if (_code_challenge_method_supported) {
    if (strSubString(_code_challenge_method_supported,
                     CODE_CHALLENGE_METHOD_S256)) {
      account_setCodeChallengeMethod(account, CODE_CHALLENGE_METHOD_S256);
    } else if (strSubString(_code_challenge_method_supported,
                            CODE_CHALLENGE_METHOD_PLAIN)) {
      account_setCodeChallengeMethod(account, CODE_CHALLENGE_METHOD_PLAIN);
    }
    secFree(_code_challenge_method_supported);
  }
  agent_log(DEBUG, "Successfully retrieved endpoints.");
  return OIDC_SUCCESS;
}
