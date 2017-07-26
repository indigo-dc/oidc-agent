#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <syslog.h>
#include <time.h>

#include "http.h"
#include "config.h"
#include "oidc.h"
#include "ipc_prompt.h"

int getAccessToken(int provider) {
  if (conf_getRefreshToken(provider)!=NULL && strcmp("", conf_getRefreshToken(provider))!=0) 
    if(tryRefreshFlow()==0)
      return 0;
  syslog(LOG_AUTHPRIV|LOG_NOTICE, "No valid refresh_token found for this client.\n");
  if(tryPasswordFlow()==0)
    return 0;
  exit(EXIT_FAILURE);

}

int tryRefreshFlow(int provider) {
  refreshFlow(0);
  if(NULL==conf_getAccessToken(provider))
    return 1;
#ifdef TOKEN_FILE
  writeToFile(TOKEN_FILE, conf_getAccessToken(provider));
#endif
#ifdef ENV_VAR
  setenv(ENV_VAR, conf_getAccessToken(provider),1);
#endif
  return 0;
}

int tryPasswordFlow(int provider) {
  if(conf_getUsername(provider)==NULL || strcmp("", conf_getUsername(provider))==0)
    conf_setUsername(provider, getUserInput("No username specified. Enter username for client: "));
  char* password = getUserInput("Enter password for client: ");
  if(passwordFlow(provider, password)!=0 || NULL==conf_getAccessToken(provider)) {
    free(password);
    syslog(LOG_AUTHPRIV|LOG_EMERG, "Could not get an access_token\n");
    return 1;
  }
  free(password);
#ifdef TOKEN_FILE
  writeToFile(TOKEN_FILE, conf_getAccessToken(provider));
#endif
#ifdef ENV_VAR
  setenv(ENV_VAR, conf_getAccessToken(provider),1);
#endif
  return 0;
}


int refreshFlow(int provider_i) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG,"Doing RefreshFlow\n");
  const char* format = "client_id=%s&client_secret=%s&grant_type=refresh_token&refresh_token=%s";
  char* data = calloc(sizeof(char),strlen(format)-3*2+strlen(conf_getClientSecret(provider_i))+strlen(conf_getClientId(provider_i))+strlen(conf_getRefreshToken(provider_i))+1);
  sprintf(data, format, conf_getClientId(provider_i), conf_getClientSecret(provider_i), conf_getRefreshToken(provider_i));
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Data to send: %s",data);
  char* res = httpsPOST(conf_getTokenEndpoint(provider_i), data);
  free(data);
  struct key_value pairs[2];
  pairs[0].key = "access_token";
  pairs[1].key = "expires_in";
  pairs[0].value = NULL;
  pairs[1].value = NULL;
  if (getJSONValues(res, pairs, sizeof(pairs)/sizeof(pairs[0]))<0) {
    syslog(LOG_AUTHPRIV|LOG_EMERG, "Error while parsing json\n");
    free(res);
    exit(EXIT_FAILURE);
  }
  if(NULL!=pairs[1].value) {
    conf_setTokenExpiresAt(provider_i, time(NULL)+atoi(pairs[1].value));
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "expires_at is: %lu\n",conf_getTokenExpiresAt(provider_i));
    free(pairs[1].value);
  }
  if(NULL==pairs[0].value) {
    char* error = getJSONValue(res, "error");
    char* errormessage = getJSONValue(res, "error_description");
    if(strcmp("invalid_client",error)==0) {
      syslog(LOG_AUTHPRIV|LOG_EMERG, "Client configuration not correct: %s. Please fix client configuration and try again.\n",errormessage);
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
  conf_setAccessToken(provider_i, pairs[0].value);
  return 0;
}

int passwordFlow(int provider_i, const char* password) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG,"Doing PasswordFlow\n");
  const char* format = "client_id=%s&client_secret=%s&grant_type=password&username=%s&password=%s";
  char* data = calloc(sizeof(char),strlen(format)-4*2+strlen(conf_getClientSecret(provider_i))+strlen(conf_getClientId(provider_i))+strlen(conf_getUsername(provider_i))+strlen(password)+1);
  sprintf(data, format, conf_getClientId(provider_i), conf_getClientSecret(provider_i), conf_getUsername(provider_i), password);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Data to send: %s",data);
  char* res = httpsPOST(conf_getTokenEndpoint(provider_i), data);
  free(data);
  struct key_value pairs[3];
  pairs[0].key = "access_token";
  pairs[1].key = "refresh_token";
  pairs[2].key = "expires_in";
  pairs[0].value = NULL;
  pairs[1].value = NULL;
  pairs[2].value = NULL;
  if (getJSONValues(res, pairs, sizeof(pairs)/sizeof(pairs[0]))<0) {
    syslog(LOG_AUTHPRIV|LOG_EMERG, "Error while parsing json\n");
    free(res);
    exit(EXIT_FAILURE);
  }
  if(NULL!=pairs[2].value) {
    conf_setTokenExpiresAt(provider_i,time(NULL)+atoi(pairs[2].value));
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "expires_at is: %lu\n",conf_getTokenExpiresAt(provider_i));
    free(pairs[2].value);
  }
  if(NULL==pairs[0].value) {
    char* error = getJSONValue(res, "error");
    char* errormessage = getJSONValue(res, "error_description");
    if(strcmp("invalid_client",error)==0) {
      syslog(LOG_AUTHPRIV|LOG_EMERG, "Client configuration not correct: %s. Please fix client configuration and try again.\n",errormessage);
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
  conf_setAccessToken(provider_i, pairs[0].value);
  if(NULL!=pairs[1].value) 
    conf_setRefreshToken(provider_i, pairs[1].value);
  return 0;
}

int tokenIsValidForSeconds(int provider, time_t min_valid_period) {
  time_t now = time(NULL);
  time_t expires_at = conf_getTokenExpiresAt(provider);
  return expires_at-now>0 && expires_at-now>min_valid_period;
}
