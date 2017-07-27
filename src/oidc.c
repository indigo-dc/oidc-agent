#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <syslog.h>

#include "oidc.h"
#include "http.h"
#include "config.h"
#include "ipc_prompt.h"
#include "file_io.h"

#define TOKEN_FILE "/access_token"
#define ENV_VAR "OIDC_TOKEN"

#define MAX_PASS_TRIES 3

/** @fn int getAccessToken(int provider)
 * @brief issues an access token
 * @param provider the index identifying the provider
 * @return 0 on success; 1 otherwise
 */
int getAccessToken(int provider) {
  if(tryRefreshFlow(provider)==0)
    return 0;
  syslog(LOG_AUTHPRIV|LOG_NOTICE, "No valid refresh_token found for this client.\n");
  if(tryPasswordFlow(provider)==0)
    return 0;
  return 1;
}

/** @fn int tryRefreshFlow(int provider)
 * @brief tries to issue an access token for the specified provider by using the
 * refresh flow
 * @param provider the index identifying the provider
 * @return 0 on success; 1 otherwise
 */
int tryRefreshFlow(int provider) {
  if(!isValid(conf_getRefreshToken(provider)))
    return 1;
  refreshFlow(provider);
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

/** @fn int tryPasswordFlow(int provider)
 * @brief tries to issue an access token by using the password flow. The user
 * might be prompted for his username and password
 * @param provider the index identifying the provider
 * @return 0 on success; 1 otherwise
 */
int tryPasswordFlow(int provider) {
  int usedEncryptedPassword = 0;
  int usedSavedUsername = 1;
  if(!isValid(conf_getUsername(provider))) {
    conf_setUsername(provider, getUserInput("No username specified. Enter username for provider: ", 0));
    usedSavedUsername = 0;
  }
  char* password = NULL;
  if(conf_getEncryptionMode(provider) && conf_hasEncryptedPassword(provider)) {
    // If there's an encrypted password, we prompt the user up to MAX_PASS_TRIES
    // times for the encryption password
    int i;
    for (i=0;i<MAX_PASS_TRIES;i++) {
      char* encryptionPassword = getUserInput("Enter encryption password: ",1);
      password = conf_getDecryptedPassword(provider, encryptionPassword);
      memset(encryptionPassword, 0, strlen(encryptionPassword));
      if(password!=NULL) {
        usedEncryptedPassword = 1;
        break;
      }
    } 
  }

  // We try the password flow up to MAX_PASS_TRIES times
  int i;
  for(i=0;i<MAX_PASS_TRIES;i++) {
    if(i>0 && !usedSavedUsername) // If we prompted the user for his username and password flow failed at least once, it might be because of the username
      conf_setUsername(provider, getUserInput("Enter username for provider: ", 0));
    if (password==NULL) // Only prompt the user for his password, if it was not encrypted
      password = getUserInput("Enter password for provider: ", 1);
    if(passwordFlow(provider, password)!=0 || NULL==conf_getAccessToken(provider)) {
      memset(password, 0, strlen(password));
      free(password);
      password = NULL;
      usedEncryptedPassword = 0;
      if(i!=MAX_PASS_TRIES-1) {
        syslog(LOG_AUTHPRIV|LOG_NOTICE, "Could not get an access_token on try #%d\n",i);
      } else { // reached MAX_PASS_TRIES
        syslog(LOG_AUTHPRIV|LOG_ALERT, "Could not get an access_token!");
        return 1;
      }
    } else { // password flow was succesfull
      break;
    }
  }
  if(conf_getEncryptionMode() && !usedEncryptedPassword) { // if encrpytion is enabled and we couldn't use the stored password, we will store it
    char* encryptionPassword =  getUserInput("Enter encryption password: ",1);
    conf_encryptAndSetPassword(provider, password, encryptionPassword);
    memset(encryptionPassword, 0, strlen(encryptionPassword));
    free(encryptionPassword);
  }
  memset(password, 0, strlen(password));
  free(password);
#ifdef TOKEN_FILE
  writeToFile(TOKEN_FILE, conf_getAccessToken(provider));
#endif
#ifdef ENV_VAR
  setenv(ENV_VAR, conf_getAccessToken(provider),1);
#endif
  return 0;
}

/** @fn int refreshFlow(int provider_i)
 * @brief issues an access token via refresh flow
 * @param provider_i the index identifying the provider
 * @return 0 on success; 1 otherwise
 */
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

/** @fn int passwordFlow(int provider_i, const char* password)
 * @brief issues an access token using the password flow
 * @param provider_i the index identifying the provider
 * @param password the user's password for the given provider
 * @return 0 on success; 1 otherwise
 */
int passwordFlow(int provider_i, const char* password) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG,"Doing PasswordFlow\n");
  const char* format = "client_id=%s&client_secret=%s&grant_type=password&username=%s&password=%s";
  char* data = calloc(sizeof(char),strlen(format)-4*2+strlen(conf_getClientSecret(provider_i))+strlen(conf_getClientId(provider_i))+strlen(conf_getUsername(provider_i))+strlen(password)+1);
  sprintf(data, format, conf_getClientId(provider_i), conf_getClientSecret(provider_i), conf_getUsername(provider_i), password);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Data to send: %s",data);
  char* res = httpsPOST(conf_getTokenEndpoint(provider_i), data);
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

/** @fn tokenIsValidforSeconds(int provider, time_t min_valid_period)
 * @brief checks if the access token for a provider is at least valid for a
 * given period of time
 * @param provider identifies the provider whose access token should be checked
 * @param min_valid_period the period of time the access token should be valid
 * (at least)
 * @return 1 if the access_token is valid for the given time; 0 if not.
 */
int tokenIsValidForSeconds(int provider, time_t min_valid_period) {
  time_t now = time(NULL);
  time_t expires_at = conf_getTokenExpiresAt(provider);
  return expires_at-now>0 && expires_at-now>min_valid_period;
}
