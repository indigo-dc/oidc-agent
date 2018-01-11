#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <syslog.h>

#include "oidc.h"
#include "http.h"
#include "oidc_utilities.h"
#include "oidc_error.h"
#include "settings.h"
#include "httpserver.h"


/** @fn oidc_error_t retrieveAccessToken(struct oidc_account* p, time_t min_valid_period)
 * @brief issues an access token
 * @param p a pointer to the account for whom an access token should be issued
 * @param min_valid_period the minium period of time the access token should be
 * valid in seconds
 * @return 0 on success; 1 otherwise
 */
oidc_error_t retrieveAccessToken(struct oidc_account* p, time_t min_valid_period) {
  if(min_valid_period!=FORCE_NEW_TOKEN && isValid(account_getAccessToken(*p)) && tokenIsValidForSeconds(*p, min_valid_period)) {
    return OIDC_SUCCESS;
  }
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "No acces token found that is valid long enough");
  if(tryRefreshFlow(p)==OIDC_SUCCESS) {
    return OIDC_SUCCESS;
  }
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "No valid refresh_token found for account %s.\n", account_getName(*p));
  if(tryPasswordFlow(p)==OIDC_SUCCESS) {
    return OIDC_SUCCESS;
  }
  return oidc_errno;
}

/** @fn oidc_error_t retrieveAccessTokenRefreshFlowOnly(struct oidc_account* p, time_t min_valid_period)
 * @brief retrieves an access token only trying the refresh flow
 * @param p a pointer to the account for whom an access token should be issued
 * @param min_valid_period the minium period of time the access token should be
 * valid in seconds
 * @return 0 on success; 1 otherwise
 */
oidc_error_t retrieveAccessTokenRefreshFlowOnly(struct oidc_account* p, time_t min_valid_period) {
  if(min_valid_period!=FORCE_NEW_TOKEN && isValid(account_getAccessToken(*p)) && tokenIsValidForSeconds(*p, min_valid_period)) {
    return OIDC_SUCCESS;
  }
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "No acces token found that is valid long enough");
  if((oidc_errno = tryRefreshFlow(p))==OIDC_SUCCESS) {
    return OIDC_SUCCESS;
  }
  return oidc_errno;
}


/** @fn oidc_error_t tryRefreshFlow(struct oidc_account* p)
 * @brief tries to issue an access token for the specified account by using the
 * refresh flow
 * @param p a pointer to the account for whom an access token should be issued
 * @return 0 on success; 1 otherwise
 */
oidc_error_t tryRefreshFlow(struct oidc_account* p) {
  if(!isValid(account_getRefreshToken(*p))) {
    return OIDC_ENOREFRSH;
  }
  return refreshFlow(p);
}

/** @fn oidc_error_t tryPasswordFlow(struct oidc_account* p)
 * @brief tries to issue an access token by using the password flow. The user
 * might be prompted for his username and password
 * @param p a pointer to the account for whom an access token should be issued
 * @return 0 on success; 1 otherwise
 */
oidc_error_t tryPasswordFlow(struct oidc_account* p) {
  if(!isValid(account_getUsername(*p)) || !isValid(account_getPassword(*p))) {
    oidc_errno = OIDC_ECRED;
    return oidc_errno;
  }
  return passwordFlow(p);
}

/** @fn oidc_error_t refreshFlow(struct oidc_account* p)
 * @brief issues an access token via refresh flow
 * @param p a pointer to the account for whom an access token should be issued
 * @return 0 on success; 1 otherwise
 */
oidc_error_t refreshFlow(struct oidc_account* p) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG,"Doing RefreshFlow\n");
  const char* format = "client_id=%s&client_secret=%s&grant_type=refresh_token&refresh_token=%s";
  char* data = oidc_sprintf(format, account_getClientId(*p), account_getClientSecret(*p), account_getRefreshToken(*p));
  if(data == NULL) {
    return oidc_errno;
  }
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Data to send: %s",data);
  char* res = httpsPOST(account_getTokenEndpoint(*p), data, NULL, account_getCertPath(*p), NULL, NULL);
  clearFreeString(data);
  if(NULL==res) {
    return oidc_errno;
  }
  struct key_value pairs[3];
  pairs[0].key = "access_token";
  pairs[1].key = "expires_in";
  pairs[2].key = "refresh_token";
  pairs[0].value = NULL;
  pairs[1].value = NULL;
  pairs[2].value = NULL;
  if(getJSONValues(res, pairs, sizeof(pairs)/sizeof(pairs[0]))<0) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "Error while parsing json\n");
    clearFreeString(res);
    return oidc_errno;
  }
  if(NULL!=pairs[1].value) {
    account_setTokenExpiresAt(p, time(NULL)+atoi(pairs[1].value));
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "expires_at is: %lu\n",account_getTokenExpiresAt(*p));
    clearFreeString(pairs[1].value);
  }
  if(NULL==pairs[0].value) {
    char* error = getJSONValue(res, "error");
    char* errormessage = getJSONValue(res, "error_description");
    syslog(LOG_AUTHPRIV|LOG_CRIT, "%s\n", errormessage ? errormessage : error);
    oidc_seterror(errormessage ? errormessage : error);
    clearFreeString(error);
    clearFreeString(errormessage);
    clearFreeString(res);
    oidc_errno = OIDC_EOIDC;
    return OIDC_EOIDC;
  }
  if(isValid(pairs[2].value) && strcmp(account_getRefreshToken(*p), pairs[2].value)!=0) {
    syslog(LOG_AUTHPRIV|LOG_WARNING, "WARNING: Received new refresh token from OIDC Account. It's most likely that the old one was therefore revoked. We did not save the new refresh token. You may want to revoke it. You have to run oidc-gen again.");
  }
  clearFreeString(pairs[2].value);
  clearFreeString(res);
  account_setAccessToken(p, pairs[0].value);
  return OIDC_SUCCESS;
}

//TODO refactor passwordflow and refreshFlow. There are some quite big
//duplicated parts

/** @fn oidc_error_t passwordFlow(struct oidc_account* p)
 * @brief issues an access token using the password flow
 * @param p a pointer to the account for whom an access token should be issued
 * @return 0 on success; 1 otherwise
 */
oidc_error_t passwordFlow(struct oidc_account* p) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG,"Doing PasswordFlow\n");
  const char* format = "client_id=%s&client_secret=%s&grant_type=password&username=%s&password=%s";
  char* data = oidc_sprintf(format, account_getClientId(*p), account_getClientSecret(*p), account_getUsername(*p), account_getPassword(*p));
  if(data == NULL) {
    return oidc_errno;
  }
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Data to send: %s",data);
  char* res = httpsPOST(account_getTokenEndpoint(*p), data, NULL, account_getCertPath(*p), NULL, NULL);
  clearFreeString(data);
  if(res==NULL) {
    return oidc_errno;
  }
  struct key_value pairs[3];
  pairs[0].key = "access_token";
  pairs[1].key = "refresh_token";
  pairs[2].key = "expires_in";
  pairs[0].value = NULL;
  pairs[1].value = NULL;
  pairs[2].value = NULL;
  if(getJSONValues(res, pairs, sizeof(pairs)/sizeof(pairs[0]))<0) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "Error while parsing json\n");
    clearFreeString(res);
    return oidc_errno;
  }
  if(NULL!=pairs[2].value) {
    account_setTokenExpiresAt(p,time(NULL)+atoi(pairs[2].value));
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "expires_at is: %lu\n",account_getTokenExpiresAt(*p));
    clearFreeString(pairs[2].value);
  }
  if(NULL==pairs[0].value) {
    char* error = getJSONValue(res, "error");
    char* errormessage = getJSONValue(res, "error_description");
    syslog(LOG_AUTHPRIV|LOG_CRIT, "%s\n", errormessage ? errormessage : error);
    oidc_seterror(errormessage ? errormessage : error);
    clearFreeString(error);
    clearFreeString(errormessage);
    clearFreeString(res);
    oidc_errno = OIDC_EOIDC;
    return OIDC_EOIDC;
  }
  clearFreeString(res);
  account_setAccessToken(p, pairs[0].value);
  if(NULL!=pairs[1].value)  {
    account_setRefreshToken(p, pairs[1].value);
  }
  return OIDC_SUCCESS;
}

/** @fn int tokenIsValidforSeconds(struct oidc_account p, time_t min_valid_period)
 * @brief checks if the access token for a account is at least valid for the
 * given period of time
 * @param p the account whose access token should be checked
 * @param min_valid_period the period of time the access token should be valid
 * (at least)
 * @return 1 if the access_token is valid for the given time; 0 if not.
 */
int tokenIsValidForSeconds(struct oidc_account p, time_t min_valid_period) {
  time_t now = time(NULL);
  time_t expires_at = account_getTokenExpiresAt(p);
  return expires_at-now>0 && expires_at-now>min_valid_period;
}

oidc_error_t revokeToken(struct oidc_account* account) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Performing Token revocation flow");
  if(!isValid(account_getRevocationEndpoint(*account))) {
    oidc_seterror("Token revocation is not supported by this account.");
    oidc_errno = OIDC_EERROR;
    return oidc_errno;
  }
  const char* const fmt = "token_type_hint=refresh_token&token=%s";
  char* data = oidc_sprintf(fmt, account_getRefreshToken(*account));
  if(data == NULL) {
    return oidc_errno;
  }
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Data to send: %s",data);
  char* res = httpsPOST(account_getRevocationEndpoint(*account), data, NULL, account_getCertPath(*account), account_getClientId(*account), account_getClientSecret(*account));
  clearFreeString(data);
  if(isValid(res) && json_hasKey(res, "error")) {
    char* error = getJSONValue(res, "error_description");
    if(error==NULL) {
      error = getJSONValue(res, "error");
    }
    oidc_errno = OIDC_EOIDC;
    oidc_seterror(error);
    clearFreeString(error);
  }
  clearFreeString(res);

  syslog(LOG_AUTHPRIV|LOG_DEBUG, "errno is %d and message is %s", oidc_errno, oidc_serror());
  if(oidc_errno==OIDC_SUCCESS) {
    account_setRefreshToken(account, NULL);
  }

  return oidc_errno;
}

char* dynamicRegistration(struct oidc_account* account, int useGrantType) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Performing dynamic Registration flow");
  if(!isValid(account_getRegistrationEndpoint(*account))) {
    oidc_seterror("Dynamic registration is not supported by this account.");
    oidc_errno = OIDC_EERROR;
    return NULL;
  }
  char* client_name = oidc_sprintf("oidc-agent:%s", account_getName(*account));
  if(client_name == NULL) {
    return NULL;
  }

  char* json = calloc(sizeof(char), 2+1);
  strcpy(json, "{}");
  json = json_addStringValue(json, "application_type", "web");
  json = json_addStringValue(json, "client_name", client_name);
  clearFreeString(client_name);
  json = json_addStringValue(json, "response_types", "code");
  if(useGrantType) {
    json = json_addValue(json, "grant_types", "[\"password\", \"refresh_token\", \"authorization_code\"]");
  }
  json = json_addValue(json, "scope", "\"openid email profile offline_access\"");
  char** redirect_uris = calloc(sizeof(char*), 3);
  char* redirect_uris_json = calloc(sizeof(char), 2+1);
  strcpy(redirect_uris_json, "[]");
  redirect_uris[0] = portToUri(HTTP_DEFAULT_PORT);
  json_arrAdd(redirect_uris_json, redirect_uris[0]);
  redirect_uris[0] = portToUri(getRandomPort());
  json_arrAdd(redirect_uris_json, redirect_uris[0]);
  redirect_uris[0] = portToUri(HTTP_FALLBACK_PORT);
  json_arrAdd(redirect_uris_json, redirect_uris[0]);
  json = json_addValue(json, "redirect_uris", redirect_uris_json);
  clearFreeString(redirect_uris_json);


  struct curl_slist* headers = NULL;
  headers = curl_slist_append(headers, "Content-Type: application/json");
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Data to send: %s",json);
  char* res = httpsPOST(account_getRegistrationEndpoint(*account), json, headers, account_getCertPath(*account), NULL, NULL);
  curl_slist_free_all(headers);
  clearFreeString(json);
  if(res==NULL) {
    return NULL;
  }
  return res;
}

oidc_error_t codeExchange(struct oidc_account* account, const char* code) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG,"Doing Authorzation Code Flow\n");
  const char* format = "client_id=%s&client_secret=%s&grant_type=authorization_code&code=%sredirect_uri=%s";
  char* data = oidc_sprintf(format, account_getClientId(*account), account_getClientSecret(*account), code, "http://localhost:2912");
  if(data == NULL) {
    return oidc_errno;
  }
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Data to send: %s",data);
  char* res = httpsPOST(account_getTokenEndpoint(*account), data, NULL, account_getCertPath(*account), NULL, NULL);
  clearFreeString(data);
  if(res==NULL) {
    return oidc_errno;
  }
  struct key_value pairs[3];
  pairs[0].key = "access_token";
  pairs[1].key = "refresh_token";
  pairs[2].key = "expires_in";
  pairs[0].value = NULL;
  pairs[1].value = NULL;
  pairs[2].value = NULL;
  if(getJSONValues(res, pairs, sizeof(pairs)/sizeof(pairs[0]))<0) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "Error while parsing json\n");
    clearFreeString(res);
    return oidc_errno;
  }
  if(NULL!=pairs[2].value) {
    account_setTokenExpiresAt(account,time(NULL)+atoi(pairs[2].value));
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "expires_at is: %lu\n",account_getTokenExpiresAt(*account));
    clearFreeString(pairs[2].value);
  }
  if(NULL==pairs[0].value) {
    char* error = getJSONValue(res, "error");
    char* errormessage = getJSONValue(res, "error_description");
    syslog(LOG_AUTHPRIV|LOG_CRIT, "%s\n", errormessage ? errormessage : error);
    oidc_seterror(errormessage ? errormessage : error);
    clearFreeString(error);
    clearFreeString(errormessage);
    clearFreeString(res);
    oidc_errno = OIDC_EOIDC;
    return OIDC_EOIDC;
  }
  clearFreeString(res);
  account_setAccessToken(account, pairs[0].value);
  if(NULL!=pairs[1].value)  {
    account_setRefreshToken(account, pairs[1].value);
  }
  return OIDC_SUCCESS;
}

char* buildCodeFlowUri(struct oidc_account* account) {
  const char* auth_endpoint = account_getAuthorizationEndpoint(*account);
  char** redirect_uris = account_getRedirectUris(*account);
  size_t count = account_getRedirectUrisCount(*account);
  if(redirect_uris==NULL || count<=0) {
    oidc_errno = OIDC_ENOREURI;
    return NULL;
  }
  unsigned int i = 0;
  int port = getPortFromUri(redirect_uris[i]);
  while(startHttpServer(port)==NULL) {
    i++;
    if(i>=count) {
      oidc_errno = OIDC_EHTTPPORTS;
      return NULL;
    }
    port = getPortFromUri(redirect_uris[i]);
  }
  char* uri = oidc_sprintf("%s?response_type=code&client_id=%s&redirect_uri=%s&scope=%s", 
      auth_endpoint,
      account_getClientId(*account),
      redirect_uris[i],
      "openid email profile offline_access"); // TODO redirect uri, state
  return uri;
}

/** @fn oidc_error_t getEndpoints(struct oidc_account* account)
 * @brief retrieves account config from the configuration_endpoint
 * @note the configuration_endpoint has to set prior
 * @param account the account struct, will be updated with the retrieved
 * endpoints
 * @return a oidc_error status code
 */
oidc_error_t getEndpoints(struct oidc_account* account) {
  char* configuration_endpoint = oidc_strcat(account_getIssuerUrl(*account), CONF_ENDPOINT_SUFFIX);
  issuer_setConfigurationEndpoint(account_getIssuer(*account), configuration_endpoint);
  char* res = httpsGET(account_getConfigEndpoint(*account), NULL, account_getCertPath(*account));
  if(NULL==res) {
    return oidc_errno;
  }
  struct key_value pairs[4];
  pairs[0].key = "token_endpoint";
  pairs[1].key = "authorization_endpoint";
  pairs[2].key = "registration_endpoint";
  pairs[3].key = "revocation_endpoint";
  if(getJSONValues(res, pairs, sizeof(pairs)/sizeof(*pairs))<0) {
    clearFreeString(res);
    return oidc_errno;
  }
  clearFreeString(res);

  if(pairs[0].value==NULL) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "Could not get token_endpoint");
    clearFreeString(pairs[1].value);
    clearFreeString(pairs[2].value);
    clearFreeString(pairs[3].value);
    oidc_seterror("Could not get token_endpoint from the configuration_endpoint. This could be because of a network issue. But it's more likely that your issuer is not correct.");
    oidc_errno = OIDC_EERROR;
    return oidc_errno;
  }
  issuer_setTokenEndpoint(account_getIssuer(*account), pairs[0].value);
  issuer_setAuthorizationEndpoint(account_getIssuer(*account), pairs[1].value);
  issuer_setRegistrationEndpoint(account_getIssuer(*account), pairs[2].value);
  issuer_setRevocationEndpoint(account_getIssuer(*account), pairs[3].value);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Successfully retrieved endpoints.");
  return OIDC_SUCCESS;

}

