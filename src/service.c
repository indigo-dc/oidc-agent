#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "oidc.h"

#define CONFIGFILE "config.conf"

#define LOG_DEBUG


int main(int argc, char** argv) {
  readConfig(CONFIGFILE);
  char* access_token = refreshToken();
  printf("\naccess_token: %s\n\n", access_token);
  free(access_token);
  return 0;
}

