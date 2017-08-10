#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <ctype.h>

#include "../../src/provider.h"
#include "../../src/prompt.h"
#include "../../src/ipc.h"

#define OIDC_SOCK_ENV_NAME "OIDC_SOCK"

char* provider = NULL;

/** @fn void parseOpt(int argc, char* const* argv)
 * @brief parses the command line arguments
 * @param argc the number of command line arguments
 * @param argv the command line arguments
 */
void parseOpt(int argc, char* const* argv) {
  int c;
  while ((c = getopt (argc, argv, "p:")) != -1)
    switch (c) {
      case 'p':
        provider = optarg;
        break;
      case '?':
        if (optopt == 'c')
          printf("Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          printf("Unknown option `-%c'.\n", optopt);
        else
          printf("Unknown option character `\\x%x'.\n", optopt);
        exit(EXIT_FAILURE);
      default:
        abort ();
    }
}

int main(int argc, char** argv) {
  parseOpt(argc, argv);
  if(provider==NULL) {
    printf("No provider specified\n");
    exit(EXIT_FAILURE);
  }
  if(!providerConfigExists(provider)) {
    printf("No provider configured with that short name\n");
    exit(EXIT_FAILURE);
  }
  struct oidc_provider* p = NULL;
  while(NULL==p) {
    char* password = promptPassword("Enter encrpytion password for provider %s: ", provider);
    p = decryptProvider(provider, password);
    free(password);
  }
  char* json_p = providerToJSON(*p);
  freeProvider(p);

  struct connection con = {0,0,0};
  if(ipc_init(&con, NULL, OIDC_SOCK_ENV_NAME, 0)!=0)
    exit(EXIT_FAILURE);
  if(ipc_connect(con)<0) {
    printf("Could not connect to oicd\n");
    exit(EXIT_FAILURE);
  }
  ipc_write(*(con.sock), "add:%s", json_p);
  free(json_p);
  char* res = ipc_read(*(con.sock));
  ipc_close(&con);
  if(NULL==res) {
    printf("An unexpected error occured. It's seems that oidcd has stopped.\n That's not good.");
    exit(EXIT_FAILURE);
  }

  struct key_value pairs[2];
  pairs[0].key = "status";
  pairs[1].key = "error";
  if(getJSONValues(res, pairs, sizeof(pairs)/sizeof(*pairs))<0) {
    printf("Could not decode json: %s\n", res);
    printf("This seems to be a bug. Please hand in a bug report.\n");
    free(res);
    exit(EXIT_FAILURE);
  }
  free(res);
  if(pairs[1].value!=NULL) {
    printf("Error: %s\n", pairs[1].value);
    free(pairs[1].value); free(pairs[0].value);
    exit(EXIT_FAILURE);
  }
  printf("%s\n", pairs[0].value);
  free(pairs[0].value);
}
