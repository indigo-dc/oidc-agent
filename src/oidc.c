#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "http.h"
#include "config.h"
#include "oidc.h"
#include "logger.h"


int refreshToken(int provider_i) {
  const char* format = "client_id=%s&client_secret=%s&grant_type=refresh_token&refresh_token=%s";
  char* data = (char*)malloc(strlen(format)-3*2+strlen(config.provider[provider_i].client_secret)+strlen(config.provider[provider_i].client_id)+strlen(config.provider[provider_i].refresh_token)+1);
  sprintf(data, format, config.provider[provider_i].client_id, config.provider[provider_i].client_secret, config.provider[provider_i].refresh_token);
  logging(DEBUG, "Data to send: %s",data);
  char* res = httpsPOST(config.provider[provider_i].token_endpoint, data);
  free(data);
  struct key_value pairs[2];
  pairs[0].key = "access_token";
  pairs[1].key = "expires_in";
  pairs[0].value = NULL;
  pairs[1].value = NULL;
  if (getJSONValues(res, pairs, sizeof(pairs)/sizeof(pairs[0]))<0) {
    fprintf(stderr, "Error while parsing json\n");
    free(res);
    exit(EXIT_FAILURE);
  }
  if(NULL!=pairs[1].value) {
    config.provider[provider_i].token_expires_in = atoi(pairs[1].value);
    logging(DEBUG, "expires_in is: %d\n",config.provider[provider_i].token_expires_in);
    free(pairs[1].value);
  }
  if(NULL==pairs[0].value) {
    char* error = getJSONValue(res, "error");
    char* errormessage = getJSONValue(res, "error_description");
    if(strcmp("invalid_client",error)==0) {
      fprintf(stderr, "ERROR: Client configuration not correct: %s\nPlease fix client configuration and try again.\n",errormessage);
      free(error);
      free(errormessage);
      free(res);
      exit(EXIT_FAILURE);
    }
    fprintf(stderr, "ERROR: %s\n", errormessage ? errormessage : error);
    free(error);
    free(errormessage);
    free(res);
    return 1;
  }
  free(res);
  free(config.provider[provider_i].access_token);
  config.provider[provider_i].access_token = pairs[0].value;
  return 0;
}

int passwordFlow(int provider_i, const char* password) {
  const char* format = "client_id=%s&client_secret=%s&grant_type=password&username=%s&password=%s";
  char* data = (char*)malloc(strlen(format)-4*2+strlen(config.provider[provider_i].client_secret)+strlen(config.provider[provider_i].client_id)+strlen(config.provider[provider_i].username)+strlen(password)+1);
  sprintf(data, format, config.provider[provider_i].client_id, config.provider[provider_i].client_secret, config.provider[provider_i].username, password);
  logging(DEBUG, "Data to send: %s",data);
  char* res = httpsPOST(config.provider[provider_i].token_endpoint, data);
  free(data);
  struct key_value pairs[3];
  pairs[0].key = "access_token";
  pairs[1].key = "refresh_token";
  pairs[2].key = "expires_in";
  pairs[0].value = NULL;
  pairs[1].value = NULL;
  pairs[2].value = NULL;
  if (getJSONValues(res, pairs, sizeof(pairs)/sizeof(pairs[0]))<0) {
    fprintf(stderr, "Error while parsing json\n");
    free(res);
    exit(EXIT_FAILURE);
  }
  if(NULL!=pairs[1].value) {
    free(config.provider[provider_i].refresh_token);
    config.provider[provider_i].refresh_token = pairs[1].value;
  }
  if(NULL!=pairs[2].value) {
    config.provider[provider_i].token_expires_in = atoi(pairs[2].value);
    logging(DEBUG, "expires_in is: %d\n",config.provider[provider_i].token_expires_in);
    free(pairs[2].value);
  }
  if(NULL==pairs[0].value) {
    char* error = getJSONValue(res, "error");
    char* errormessage = getJSONValue(res, "error_description");
    if(strcmp("invalid_client",error)==0) {
      fprintf(stderr, "ERROR: Client configuration not correct: %s\nPlease fix client configuration and try again.\n",errormessage);
      free(error);
      free(errormessage);
      free(res);
      exit(EXIT_FAILURE);
    }
    fprintf(stderr, "ERROR: %s\n", errormessage ? errormessage : error);
    free(error);
    free(errormessage);
    free(res);
    return 1;
  }
  free(res);
  free(config.provider[provider_i].access_token);
  config.provider[provider_i].access_token = pairs[0].value;
  return 0;

}
