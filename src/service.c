#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "service.h"
#include "config.h"
#include "oidc.h"


int main(int argc, char** argv) {
  readConfig();
  parseOpt(argc, argv);

  if (config.provider[provider].refresh_token!=NULL && strcmp("", config.provider[provider].refresh_token)!=0) 
    if(tryRefreshFlow()==0)
      return 0;
  printf("No valid refresh_token found for this client.\n");
  if(tryPasswordFlow()==0)
    return 0;
  return EXIT_FAILURE;
}


void parseOpt(int argc, char* const* argv) {
  int c;
  char* cvalue = NULL;
  while ((c = getopt (argc, argv, "c:r")) != -1)
    switch (c) {
      case 'c':
        cvalue = optarg;
        if(!isdigit(cvalue[0])) {
          fprintf(stderr, "Client has to be specified by number.\n");
          exit(EXIT_FAILURE);
        }
        provider = atoi(cvalue);
        if (provider>=config.provider_count) {
          fprintf(stderr, "Invalid provider specified!\nYou just configured %d provider.\n", config.provider_count);
          exit(EXIT_FAILURE);
        }
        break;
      case 'r':
        refresh = 1;
        break;
      case '?':
        if (optopt == 'c')
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,
              "Unknown option character `\\x%x'.\n",
              optopt);
        exit(EXIT_FAILURE);
      default:
        abort ();
    }
}

int tryRefreshFlow() {
  char* access_token = refreshToken(0);
  if(NULL==access_token)
    return 1;
  printf("\naccess_token: %s\n\n", access_token);
  if(refresh)
    printf("\nrefresh_token: %s\n\n", config.provider[provider].refresh_token);
  free(access_token);
  return 0;
}

int tryPasswordFlow() {
  if(config.provider[provider].username==NULL || strcmp("", config.provider[provider].username)==0)
    config.provider[provider].username = getpass("No username specified. Enter username for client: ");
  char* password = getpass("Enter password for client: ");
  char* access_token = passwordFlow(provider, password);
  free(password);
  if(NULL==access_token)
    return 1;
  printf("\naccess_token: %s\n\n", access_token);
  if(refresh)
    printf("\nrefresh_token: %s\n\n", config.provider[provider].refresh_token);
  free(access_token);
  return 0;

}

