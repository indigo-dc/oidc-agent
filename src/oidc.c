#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <syslog.h>

#include "http.h"
#include "config.h"
#include "oidc.h"


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
    conf_setTokenExpiresIn(provider_i, atoi(pairs[1].value));
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "expires_in is: %lu\n",conf_getTokenExpiresIn(provider_i));
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
    conf_setTokenExpiresIn(provider_i,atoi(pairs[2].value));
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "expires_in is: %lu\n",conf_getTokenExpiresIn(provider_i));
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
