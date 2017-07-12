#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "config.h"
#include "oidc.h"


int main(int argc, char** argv) {
  readConfig();
  int provider = 0;
  char* cvalue = NULL;
  int c;
  while ((c = getopt (argc, argv, "c:")) != -1)
    switch (c)
    {
      case 'c':
        cvalue = optarg;
        if(!isdigit(cvalue[0])) {
          fprintf(stderr, "Client has to be specified by number.\n");
          exit(EXIT_FAILURE);
        }
        provider = atoi(cvalue);
        if (provider>=config.provider_count) {
          fprintf(stderr, "Inalid provider specified\n");
          exit(EXIT_FAILURE);
        }
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

  char* access_token = NULL;
  if (config.provider[provider].refresh_token!=NULL && strcmp("", config.provider[provider].refresh_token)!=0) {
    access_token = refreshToken(0);
    if(NULL!=access_token){
      printf("\naccess_token: %s\n\n", access_token);
      free(access_token);
      return 0;
    }
  }
  char* password = getpass("No valid refresh_token found for this client. Enter password for client: ");
  access_token = passwordFlow(provider, password);
  free(password);
  if(NULL!=access_token){
      printf("\naccess_token: %s\n\n", access_token);
      free(access_token);
      return 0;
    }
  return EXIT_FAILURE;
}

