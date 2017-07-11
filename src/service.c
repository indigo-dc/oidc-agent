#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "oidc.h"


#define LOG_DEBUG

int main(int argc, char** argv) {
  readConfig();
  int provider = 0;
  if(argc>1)
    provider = atoi(argv[1]);
  if (provider>=config.provider_count) {
    fprintf(stderr, "Inalid provider specified\n");
    exit(EXIT_FAILURE);
  }
  if (config.provider[provider].refresh_token!=NULL && strcmp("", config.provider[provider].refresh_token)!=0) {
    char* access_token = refreshToken(0);
    printf("\naccess_token: %s\n\n", access_token);
    free(access_token);
  } else {
    char* password = getpass("No refresh_token found for this client. Enter password for client: ");
    char* access_token = passwordFlow(provider, password);
    free(password);
    printf("\naccess_token: %s\n\n", access_token);
    free(access_token);
  }
  return 0;
}

