#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http.h"
#include "config.h"
#include "oidc.h"


const char* refreshToken(int provider_i) {
  const char* format = "client_id=%s&client_secret=%s&grant_type=refresh_token&refresh_token=%s";
  char* data = (char*)malloc(strlen(format)-3*2+strlen(config.provider[provider_i].client_secret)+strlen(config.provider[provider_i].client_id)+strlen(config.provider[provider_i].refresh_token));;
  sprintf(data, format, config.provider[provider_i].client_id, config.provider[provider_i].client_secret, config.provider[provider_i].refresh_token);
  printf("Data to send: %s\n",data);
  const char* res = httpsPOST(config.provider[provider_i].token_endpoint, data);
  char* access_token = getJSONValue(res, "access_token");
  return access_token;
}
