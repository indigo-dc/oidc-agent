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


/** @fn oidc_error_t retrieveAccessToken(struct oidc_provider* p, time_t min_valid_period)
 * @brief issues an access token
 * @param p a pointer to the provider for whom an access token should be issued
 * @param min_valid_period the minium period of time the access token should be
 * valid in seconds
 * @return 0 on success; 1 otherwise
 */
oidc_error_t retrieveAccessToken(struct oidc_provider* p, time_t min_valid_period) {
  if(min_valid_period!=FORCE_NEW_TOKEN && isValid(provider_getAccessToken(*p)) && tokenIsValidForSeconds(*p, min_valid_period)) {
    return OIDC_SUCCESS;
  }
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "No acces token found that is valid long enough");
  if(tryRefreshFlow(p)==OIDC_SUCCESS) {
    return OIDC_SUCCESS;
  }
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "No valid refresh_token found for provider %s.\n", provider_getName(*p));
  if(tryPasswordFlow(p)==OIDC_SUCCESS) {
    return OIDC_SUCCESS;
  }
  return oidc_errno;
}

/** @fn oidc_error_t retrieveAccessTokenRefreshFlowOnly(struct oidc_provider* p, time_t min_valid_period)
 * @brief retrieves an access token only trying the refresh flow
 * @param p a pointer to the provider for whom an access token should be issued
 * @param min_valid_period the minium period of time the access token should be
 * valid in seconds
 * @return 0 on success; 1 otherwise
 */
oidc_error_t retrieveAccessTokenRefreshFlowOnly(struct oidc_provider* p, time_t min_valid_period) {
  if(min_valid_period!=FORCE_NEW_TOKEN && isValid(provider_getAccessToken(*p)) && tokenIsValidForSeconds(*p, min_valid_period)) {
    return OIDC_SUCCESS;
  }
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "No acces token found that is valid long enough");
  if((oidc_errno = tryRefreshFlow(p))==OIDC_SUCCESS) {
    return OIDC_SUCCESS;
  }
  return oidc_errno;
}


/** @fn oidc_error_t tryRefreshFlow(struct oidc_provider* p)
 * @brief tries to issue an access token for the specified provider by using the
 * refresh flow
 * @param p a pointer to the provider for whom an access token should be issued
 * @return 0 on success; 1 otherwise
 */
oidc_error_t tryRefreshFlow(struct oidc_provider* p) {
  if(!isValid(provider_getRefreshToken(*p))) {
    return OIDC_ENOREFRSH;
  }
  return refreshFlow(p);
}

/** @fn oidc_error_t tryPasswordFlow(struct oidc_provider* p)
 * @brief tries to issue an access token by using the password flow. The user
 * might be prompted for his username and password
 * @param p a pointer to the provider for whom an access token should be issued
 * @return 0 on success; 1 otherwise
 */
oidc_error_t tryPasswordFlow(struct oidc_provider* p) {
  if(!isValid(provider_getUsername(*p)) || !isValid(provider_getPassword(*p))) {
    oidc_errno = OIDC_ECRED;
    return oidc_errno;
  }
  return passwordFlow(p);
}

/** @fn oidc_error_t refreshFlow(struct oidc_provider* p)
 * @brief issues an access token via refresh flow
 * @param p a pointer to the provider for whom an access token should be issued
 * @return 0 on success; 1 otherwise
 */
oidc_error_t refreshFlow(struct oidc_provider* p) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG,"Doing RefreshFlow\n");
  const char* format = "client_id=%s&client_secret=%s&grant_type=refresh_token&refresh_token=%s";
  char* data = calloc(sizeof(char),strlen(format)-3*2+strlen(provider_getClientSecret(*p))+strlen(provider_getClientId(*p))+strlen(provider_getRefreshToken(*p))+1);
  sprintf(data, format, provider_getClientId(*p), provider_getClientSecret(*p), provider_getRefreshToken(*p));
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Data to send: %s",data);
  char* res = httpsPOST(provider_getTokenEndpoint(*p), data, NULL, provider_getCertPath(*p));
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
    provider_setTokenExpiresAt(p, time(NULL)+atoi(pairs[1].value));
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "expires_at is: %lu\n",provider_getTokenExpiresAt(*p));
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
  if(isValid(pairs[2].value) && strcmp(provider_getRefreshToken(*p), pairs[2].value)!=0) {
    syslog(LOG_AUTHPRIV|LOG_WARNING, "WARNING: Received new refresh token from OIDC Provider. It's most likely that the old one was therefore revoked. We did not save the new refresh token. You may want to revoke it. You have to run oidc-gen again.");
  }
  clearFreeString(pairs[2].value);
  clearFreeString(res);
  provider_setAccessToken(p, pairs[0].value);
  return OIDC_SUCCESS;
}

//TODO refactor passwordflow and refreshFlow. There are some quite big
//duplicated parts

/** @fn oidc_error_t passwordFlow(struct oidc_provider* p)
 * @brief issues an access token using the password flow
 * @param p a pointer to the provider for whom an access token should be issued
 * @return 0 on success; 1 otherwise
 */
oidc_error_t passwordFlow(struct oidc_provider* p) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG,"Doing PasswordFlow\n");
  const char* format = "client_id=%s&client_secret=%s&grant_type=password&username=%s&password=%s";
  char* data = calloc(sizeof(char),strlen(format)-4*2+strlen(provider_getClientSecret(*p))+strlen(provider_getClientId(*p))+strlen(provider_getUsername(*p))+strlen(provider_getPassword(*p))+1);
  sprintf(data, format, provider_getClientId(*p), provider_getClientSecret(*p), provider_getUsername(*p), provider_getPassword(*p));
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Data to send: %s",data);
  char* res = httpsPOST(provider_getTokenEndpoint(*p), data, NULL, provider_getCertPath(*p));
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
    provider_setTokenExpiresAt(p,time(NULL)+atoi(pairs[2].value));
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "expires_at is: %lu\n",provider_getTokenExpiresAt(*p));
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
  provider_setAccessToken(p, pairs[0].value);
  if(NULL!=pairs[1].value)  {
    provider_setRefreshToken(p, pairs[1].value);
  }
  return OIDC_SUCCESS;
}

/** @fn int tokenIsValidforSeconds(struct oidc_provider p, time_t min_valid_period)
 * @brief checks if the access token for a provider is at least valid for the
 * given period of time
 * @param p the provider whose access token should be checked
 * @param min_valid_period the period of time the access token should be valid
 * (at least)
 * @return 1 if the access_token is valid for the given time; 0 if not.
 */
int tokenIsValidForSeconds(struct oidc_provider p, time_t min_valid_period) {
  time_t now = time(NULL);
  time_t expires_at = provider_getTokenExpiresAt(p);
  return expires_at-now>0 && expires_at-now>min_valid_period;
}

char* dynamicRegistration(struct oidc_provider* provider, int useGrantType) {
  char* client_name = calloc(sizeof(char), strlen(provider_getName(*provider))+strlen("oidc-agent:")+1);
  sprintf(client_name, "oidc-agent:%s", provider_getName(*provider));

  char* json = calloc(sizeof(char), 2+1);
  strcpy(json, "{}");
  json = json_addStringValue(json, "application_type", "web");
  json = json_addStringValue(json, "client_name", client_name);
  clearFreeString(client_name);
  json = json_addStringValue(json, "response_types", "code");
  if(useGrantType) {
  json = json_addValue(json, "grant_types", "[\"password\", \"refresh_token\", \"authorization_code\"]");
  }
  json = json_addValue(json, "redirect_uris", "[\"http://localhost:8080\", \"http://localhost:2912\", \"http://localhost:2408\"]");
  json = json_addValue(json, "scope", "\"openid email profile offline_access\"");


  

  struct curl_slist* headers = NULL;
  headers = curl_slist_append(headers, "Content-Type: application/json");
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Data to send: %s",json);
  char* res = httpsPOST(provider_getRegistrationEndpoint(*provider), json, headers, provider_getCertPath(*provider));
  curl_slist_free_all(headers);
  clearFreeString(json);
  if(res==NULL) {
    return NULL;
  }

  // struct key_value pairs[4];
  // pairs[0].key = "client_id";
  // pairs[1].key = "client_secret";
  // pairs[2].key = "error";
  // pairs[3].key = "error_message";
  // if(getJSONValues(res, pairs, sizeof(pairs)/sizeof(*pairs))<0) {
  //   syslog(LOG_AUTHPRIV|LOG_ALERT, "Error while parsing json\n");
  //   clearFreeString(res);
  //   return NULL;
  // }
  //
  // if(pairs[2].value!=NULL) {
  //   syslog(LOG_AUTHPRIV|LOG_ALERT, "Error registering client: %s: %s", pairs[2].value, pairs[3].value ? pairs[3].value : "");
  //   oidc_seterror(pairs[3].value ? pairs[3].value : pairs[2].value);
  //   oidc_errno = OIDC_EOIDC;
  //   clearFreeString(pairs[0].value);
  //   clearFreeString(pairs[1].value);
  //   clearFreeString(pairs[2].value);
  //   clearFreeString(pairs[3].value);
  //   return NULL;
  // }
  //   clearFreeString(pairs[2].value);
  //   clearFreeString(pairs[3].value);
  // if(pairs[0].value==NULL || pairs[1].value==NULL) {
  //   syslog(LOG_AUTHPRIV|LOG_ALERT, "Error registering client: Did not find client_id and client_secret");
  //   oidc_seterror("Error registering client: Did not find client_id and client_secret");
  //   oidc_errno = OIDC_EOIDC;
  //   clearFreeString(pairs[0].value);
  //   clearFreeString(pairs[1].value);
  //   return NULL;
  // }
  // provider_setClientId(provider, pairs[0].value);
  // provider_setClientSecret(provider, pairs[1].value);
  return res;
}


/** @fn oidc_error_t getEndpoints(struct oidc_provider* provider)
 * @brief retrieves provider config from the configuration_endpoint
 * @note the configuration_endpoint has to set prior
 * @param provider the provider struct, will be updated with the retrieved
 * endpoints
 * @return a oidc_error status code
 */
oidc_error_t getEndpoints(struct oidc_provider* provider) {
  char* res = httpsGET(provider_getConfigEndpoint(*provider), NULL, provider_getCertPath(*provider));
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
  provider_setTokenEndpoint(provider, pairs[0].value);
  provider_setAuthorizationEndpoint(provider, pairs[1].value);
  provider_setRegistrationEndpoint(provider, pairs[2].value);
  provider_setRevocationEndpoint(provider, pairs[3].value);
  return OIDC_SUCCESS;

  }

