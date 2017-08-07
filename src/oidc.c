#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <syslog.h>

#include "oidc.h"
#include "http.h"
#include "oidc_string.h"
// #include "ipc.h"
// #include "ipc_prompt.h"
// #include "file_io.h"


/** @fn int getAccessToken(int provider)
 * @brief issues an access token
 * @param provider the index identifying the provider
 * @return 0 on success; 1 otherwise
 */
int getAccessToken(struct oidc_provider* p, time_t min_valid_period) {
  if (min_valid_period!=FORCE_NEW_TOKEN && isValid(provider_getAccessToken(*p)) && tokenIsValidForSeconds(*p, min_valid_period))
    return 0;
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "No acces token found that is valid long enough");
  if(tryRefreshFlow(p)==0)
    return 0;
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "No valid refresh_token found for provider %s.\n", provider_getName(*p));
  if(tryPasswordFlow(p)==0)
    return 0;
  return 1;
}

/** @fn int tryRefreshFlow(int provider)
 * @brief tries to issue an access token for the specified provider by using the
 * refresh flow
 * @param provider the index identifying the provider
 * @return 0 on success; 1 otherwise
 */
int tryRefreshFlow(struct oidc_provider* p) {
  if(!isValid(provider_getRefreshToken(*p)))
    return 1;
  return refreshFlow(p);
}

/** @fn int tryPasswordFlow(int provider)
 * @brief tries to issue an access token by using the password flow. The user
 * might be prompted for his username and password
 * @param provider the index identifying the provider
 * @return 0 on success; 1 otherwise
 */
int tryPasswordFlow(struct oidc_provider* p) {
  if(!isValid(provider_getUsername(*p)) || !isValid(provider_getPassword(*p)))
    return 1;
  return passwordFlow(p);
}

/** @fn int refreshFlow(int provider_i)
 * @brief issues an access token via refresh flow
 * @param provider_i the index identifying the provider
 * @return 0 on success; 1 otherwise
 */
int refreshFlow(struct oidc_provider* p) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG,"Doing RefreshFlow\n");
  const char* format = "client_id=%s&client_secret=%s&grant_type=refresh_token&refresh_token=%s";
  char* data = calloc(sizeof(char),strlen(format)-3*2+strlen(provider_getClientSecret(*p))+strlen(provider_getClientId(*p))+strlen(provider_getRefreshToken(*p))+1);
  sprintf(data, format, provider_getClientId(*p), provider_getClientSecret(*p), provider_getRefreshToken(*p));
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Data to send: %s",data);
  char* res = httpsPOST(provider_getTokenEndpoint(*p), data);
  free(data);
  struct key_value pairs[2];
  pairs[0].key = "access_token";
  pairs[1].key = "expires_in";
  pairs[0].value = NULL;
  pairs[1].value = NULL;
  if (getJSONValues(res, pairs, sizeof(pairs)/sizeof(pairs[0]))<0) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "Error while parsing json\n");
    free(res);
    return 1;
  }
  if(NULL!=pairs[1].value) {
    provider_setTokenExpiresAt(p, time(NULL)+atoi(pairs[1].value));
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "expires_at is: %lu\n",provider_getTokenExpiresAt(*p));
    free(pairs[1].value);
  }
  if(NULL==pairs[0].value) {
    char* error = getJSONValue(res, "error");
    char* errormessage = getJSONValue(res, "error_description");
    if(strcmp("invalid_client",error)==0) {
      syslog(LOG_AUTHPRIV|LOG_ALERT, "Client configuration not correct: %s. Please fix client configuration and try again.\n",errormessage);
      free(error);
      free(errormessage);
      free(res);
      return 1;
    }
    syslog(LOG_AUTHPRIV|LOG_CRIT, "%s\n", errormessage ? errormessage : error);
    free(error);
    free(errormessage);
    free(res);
    return 1;
  }
  free(res);
  provider_setAccessToken(p, pairs[0].value);
  return 0;
}

//TODO refactor passwordflow and refreshFlow. There are some quite big
//duplicated parts

/** @fn int passwordFlow(int provider_i, const char* password)
 * @brief issues an access token using the password flow
 * @param provider_i the index identifying the provider
 * @param password the user's password for the given provider
 * @return 0 on success; 1 otherwise
 */
int passwordFlow(struct oidc_provider* p) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG,"Doing PasswordFlow\n");
  const char* format = "client_id=%s&client_secret=%s&grant_type=password&username=%s&password=%s";
  char* data = calloc(sizeof(char),strlen(format)-4*2+strlen(provider_getClientSecret(*p))+strlen(provider_getClientId(*p))+strlen(provider_getUsername(*p))+strlen(provider_getPassword(*p))+1);
  sprintf(data, format, provider_getClientId(*p), provider_getClientSecret(*p), provider_getUsername(*p), provider_getPassword(*p));
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Data to send: %s",data);
  char* res = httpsPOST(provider_getTokenEndpoint(*p), data);
  memset(data, 0, strlen(data));
  free(data);
  struct key_value pairs[3];
  pairs[0].key = "access_token";
  pairs[1].key = "refresh_token";
  pairs[2].key = "expires_in";
  pairs[0].value = NULL;
  pairs[1].value = NULL;
  pairs[2].value = NULL;
  if (getJSONValues(res, pairs, sizeof(pairs)/sizeof(pairs[0]))<0) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "Error while parsing json\n");
    free(res);
    exit(EXIT_FAILURE);
  }
  if(NULL!=pairs[2].value) {
    provider_setTokenExpiresAt(p,time(NULL)+atoi(pairs[2].value));
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "expires_at is: %lu\n",provider_getTokenExpiresAt(*p));
    free(pairs[2].value);
  }
  if(NULL==pairs[0].value) {
    char* error = getJSONValue(res, "error");
    char* errormessage = getJSONValue(res, "error_description");
    if(strcmp("invalid_client",error)==0) {
      syslog(LOG_AUTHPRIV|LOG_ALERT, "Client configuration not correct: %s. Please fix client configuration and try again.\n",errormessage);
      free(error);
      free(errormessage);
      free(res);
      exit(EXIT_FAILURE);
    }
    syslog(LOG_AUTHPRIV|LOG_CRIT, "%s\n", errormessage ? errormessage : error);
    free(error);
    free(errormessage);
    free(res);
    return 1;
  }
  free(res);
  provider_setAccessToken(p, pairs[0].value);
  if(NULL!=pairs[1].value) 
    provider_setRefreshToken(p, pairs[1].value);
  return 0;
}

/** @fn tokenIsValidforSeconds(int provider, time_t min_valid_period)
 * @brief checks if the access token for a provider is at least valid for a
 * given period of time
 * @param provider identifies the provider whose access token should be checked
 * @param min_valid_period the period of time the access token should be valid
 * (at least)
 * @return 1 if the access_token is valid for the given time; 0 if not.
 */
int tokenIsValidForSeconds(struct oidc_provider p, time_t min_valid_period) {
  time_t now = time(NULL);
  time_t expires_at = provider_getTokenExpiresAt(p);
  return expires_at-now>0 && expires_at-now>min_valid_period;
}
