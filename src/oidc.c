#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http.h"
#include "config.h"
#include "oidc.h"
#include "logger.h"


 char* refreshToken(int provider_i) {
  const char* format = "client_id=%s&client_secret=%s&grant_type=refresh_token&refresh_token=%s";
  char* data = (char*)malloc(strlen(format)-3*2+strlen(config.provider[provider_i].client_secret)+strlen(config.provider[provider_i].client_id)+strlen(config.provider[provider_i].refresh_token)+1);
  sprintf(data, format, config.provider[provider_i].client_id, config.provider[provider_i].client_secret, config.provider[provider_i].refresh_token);
  logging(DEBUG, "Data to send: %s",data);
  const char* res = httpsPOST(config.provider[provider_i].token_endpoint, data);
  char* access_token = getJSONValue(res, "access_token");
  if(NULL==access_token) {
    char* error = getJSONValue(res, "error");
    char* errormessage = getJSONValue(res, "error_description");
    if(strcmp("invalid_client",error)==0) {
      fprintf(stderr, "ERROR: Client configuration not correct: %s\nPlease fix client configuration and try again.\n",errormessage);
      free(error);
      free(errormessage);
      exit(EXIT_FAILURE);
    }
    fprintf(stderr, "ERROR: %s\n", errormessage ? errormessage : error);
    free(error);
    free(errormessage);
    return NULL;

  }
  return access_token;
}

 char* passwordFlow(int provider_i, const char* password) {
  const char* format = "client_id=%s&client_secret=%s&grant_type=password&username=%s&password=%s";
  char* data = (char*)malloc(strlen(format)-4*2+strlen(config.provider[provider_i].client_secret)+strlen(config.provider[provider_i].client_id)+strlen(config.provider[provider_i].username)+strlen(password)+1);
  sprintf(data, format, config.provider[provider_i].client_id, config.provider[provider_i].client_secret, config.provider[provider_i].username, password);
  logging(DEBUG, "Data to send: %s",data);
  const char* res = httpsPOST(config.provider[provider_i].token_endpoint, data);
  char* access_token = getJSONValue(res, "access_token");
  if(NULL==access_token) {
    char* error = getJSONValue(res, "error");
    char* errormessage = getJSONValue(res, "error_description");
    if(strcmp("invalid_client",error)==0) {
      fprintf(stderr, "ERROR: Client configuration not correct: %s\nPlease fix client configuration and try again.\n",errormessage);
      free(error);
      free(errormessage);
      exit(EXIT_FAILURE);
    }
    fprintf(stderr, "ERROR: %s\n", errormessage ? errormessage : error);
    free(error);
    free(errormessage);
    return NULL;
  }
  if (config.provider[provider_i].refresh_token==NULL || strcmp("", config.provider[provider_i].refresh_token)==0)
    config.provider[provider_i].refresh_token = getJSONValue(res, "refresh_token");
  return access_token;

}
