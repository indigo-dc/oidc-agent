#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>

#include "service.h"
#include "config.h"
#include "oidc.h"
#include "logger.h"

#define TOKEN_FILE "access_token"
#define USE_ENV_VAR 0

int main(int argc, char** argv) {
  readConfig();
  parseOpt(argc, argv);
  // daemon(0,0);
  do {
    getAccessToken();
    time_t expires_at = time(NULL)+config.provider[provider].token_expires_in;
    logging(DEBUG, "token_expires_in: %d\ntoken expires at: %d\n",config.provider[provider].token_expires_in, expires_at);
    if(config.provider[provider].token_expires_in<=0)
      break;
    test();
    while(expires_at-config.provider[provider].min_valid_period>time(NULL)) 
      sleep(config.provider[provider].min_valid_period);
  } while(1);
  return EXIT_FAILURE;
}

int getAccessToken() {
  if (config.provider[provider].refresh_token!=NULL && strcmp("", config.provider[provider].refresh_token)!=0) 
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
          for(i=0;i<config.provider_count;i++) 
            if(strcmp(config.provider[i].name, cvalue)==0) {
              provider = i;
              break;
            }
          if(i>=config.provider_count) {
            fprintf(stderr, "Client name not found in config file.\n");
            exit(EXIT_FAILURE);
          }
        }
        provider = atoi(cvalue);
        if (provider>=config.provider_count) {
          fprintf(stderr, "Invalid provider specified!\nYou just configured %d provider.\n", config.provider_count);
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
  refreshToken(0);
  if(NULL==config.provider[provider].access_token)
    return 1;
  printf("\naccess_token: %s\n\n", config.provider[provider].access_token);
#ifdef TOKEN_FILE
  writeToFile(TOKEN_FILE, config.provider[provider].access_token);
#endif
#if USE_ENV_VAR
  setenv("OIDC_TOKEN", config.provider[provider].access_token,1);
#endif
  return 0;
}

int tryPasswordFlow() {
  if(config.provider[provider].username==NULL || strcmp("", config.provider[provider].username)==0)
    config.provider[provider].username = getpass("No username specified. Enter username for client: ");
  char* password = getpass("Enter password for client: ");
  if(passwordFlow(provider, password)!=0 || NULL==config.provider[provider].access_token) {
    free(password);
    return 1;
  }
  free(password);
  printf("\naccess_token: %s\n\n", config.provider[provider].access_token);
#ifdef TOKEN_FILE
  writeToFile(TOKEN_FILE, config.provider[provider].access_token);
#endif
#if USE_ENV_VAR
  setenv("OIDC_TOKEN", config.provider[provider].access_token,1);
#endif
  return 0;
}


void writeToFile(const char* filename, const char* text) {
  FILE *f = fopen(filename, "w");
  if (f == NULL)
  {
    fprintf(stderr, "Error opening file! %s\n", filename);
    exit(EXIT_FAILURE);
  }
  fprintf(f, "%s", text);

  fclose(f);
}


int test() {
  setenv("WATTSON_URL","https://watts-dev.data.kit.edu" ,0);
  // system("wattson lsprov");
  setenv("WATTSON_ISSUER", config.provider[provider].name,1);
  setenv("WATTSON_TOKEN", config.provider[provider].access_token, 1);
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
