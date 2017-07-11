#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "oidc.h"


#define LOG_DEBUG


int main(int argc, char** argv) {
  readConfig();
  const char* access_token = refreshToken(0);
  printf("\naccess_token: %s\n\n", access_token);
  free(access_token);
  return 0;
}

