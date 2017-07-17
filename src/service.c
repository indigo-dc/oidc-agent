#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <ctype.h>
#include <time.h>

#include "service.h"
#include "config.h"
#include "oidc.h"
#include "file_io.h"
#include "logger.h"

#define TOKEN_FILE "access_token"
#define USE_ENV_VAR 0

int main(int argc, char** argv) {
  readConfig();
  parseOpt(argc, argv);
  // daemon(0,0);
  do {
    getAccessToken();
    time_t expires_at = time(NULL)+conf_getTokenExpiresIn(provider);
    logging(DEBUG, "token_expires_in: %d\ntoken expires at: %d\n",conf_getTokenExpiresIn(provider), expires_at);
    if(conf_getTokenExpiresIn(provider)<=0)
      break;
    test();
    while(expires_at-conf_getMinValidPeriod(provider)>time(NULL)) 
      sleep(conf_getMinValidPeriod(provider));
  } while(1);
  return EXIT_FAILURE;
}

int getAccessToken() {
  if (conf_getRefreshToken(provider)!=NULL && strcmp("", conf_getRefreshToken(provider))!=0) 
    if(tryRefreshFlow()==0)
      return 0;
  printf("No valid refresh_token found for this client.\n");
  if(tryPasswordFlow()==0)
    return 0;
  exit(EXIT_FAILURE);

}

void parseOpt(int argc, char* const* argv) {
  int c;
  char* cvalue = NULL;
  while ((c = getopt (argc, argv, "c:")) != -1)
    switch (c) {
      case 'c':
        cvalue = optarg;
        if(!isdigit(cvalue[0])) {
          unsigned int i;
          for(i=0;i<conf_getProviderCount();i++) 
            if(strcmp(conf_getProviderName(i), cvalue)==0) {
              provider = i;
              break;
            }
          if(i>=conf_getProviderCount()) {
            fprintf(stderr, "Client name not found in config file.\n");
            exit(EXIT_FAILURE);
          }
        }
        provider = atoi(cvalue);
        if (provider>=conf_getProviderCount()) {
          fprintf(stderr, "Invalid provider specified!\nYou just configured %d provider.\n", conf_getProviderCount());
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
}

int tryRefreshFlow() {
  refreshFlow(0);
  if(NULL==conf_getAccessToken(provider))
    return 1;
  printf("\naccess_token: %s\n\n", conf_getAccessToken(provider));
#ifdef TOKEN_FILE
  writeToFile(TOKEN_FILE, conf_getAccessToken(provider));
#endif
#if USE_ENV_VAR
  setenv("OIDC_TOKEN", conf_getAccessToken(provider),1);
#endif
  return 0;
}

int tryPasswordFlow() {
  if(conf_getUsername(provider)==NULL || strcmp("", conf_getUsername(provider))==0)
    conf_setUsername(provider, getpass("No username specified. Enter username for client: "));
  char* password = getpass("Enter password for client: ");
  if(passwordFlow(provider, password)!=0 || NULL==conf_getAccessToken(provider)) {
    free(password);
    return 1;
  }
  free(password);
  printf("\naccess_token: %s\n\n", conf_getAccessToken(provider));
#ifdef TOKEN_FILE
  writeToFile(TOKEN_FILE, conf_getAccessToken(provider));
#endif
#if USE_ENV_VAR
  setenv("OIDC_TOKEN", conf_getAccessToken(provider),1);
#endif
  return 0;
}


int test() {
  setenv("WATTSON_URL","https://watts-dev.data.kit.edu" ,0);
  // system("wattson lsprov");
  setenv("WATTSON_ISSUER", conf_getProviderName(provider),1);
  setenv("WATTSON_TOKEN", conf_getAccessToken(provider), 1);
  // system("wattson lsserv");

  FILE *fp;
  char path[1035];

  fp = popen("wattson request info 2>&1", "r");
  if (fp == NULL) {
    fprintf(stderr,"Failed to run wattson\n" );
    return 1;
  }

  while (fgets(path, sizeof(path), fp) != NULL) {
    if(LOG_LEVEL>DEBUG)
      printf("%s", path);
    if(strncmp("error", path, strlen("error")) == 0) {
      if(fgets(path, sizeof(path), fp) != NULL)
        fprintf(stderr, "%s",path);
      return 1;
    }
  }
  if(pclose(fp))  {
    fprintf(stderr, "Command not found or exited with error status\n");
    return 1;
  }
  return 0;
}
