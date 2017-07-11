#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http.h"
#include "config.h"
#include "oidc.h"


const char* refreshToken() {
  const char* format = "client_id=%s&client_secret=%s&grant_type=refresh_token&refresh_token=%s&scope=openid profile";
  char* data = (char*)malloc(strlen(format)-3*2+strlen(config.client_secret)+strlen(config.client_id)+strlen(config.refresh_token));;
  sprintf(data, format, config.client_id, config.client_secret, config.refresh_token);
  printf("%s\n",data);
  const char* res = httpsPOST(config.token_endpoint, data);
  return res;
}
