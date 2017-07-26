#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <ctype.h>
#include <time.h>
#include <syslog.h>

#include "service.h"
#include "config.h"
#include "oidc.h"

#define TOKEN_FILE "/access_token"
#define ENV_VAR "OIDC_TOKEN"


int main(int argc, char** argv) {
  openlog("oidc-service", LOG_CONS|LOG_PID, LOG_AUTHPRIV);
  setlogmask(LOG_UPTO(LOG_DEBUG));
  // setlogmask(LOG_UPTO(LOG_NOTICE));
  readEncryptedConfig();
  readConfig();
  parseOpt(argc, argv);
  int pid = fork();
  if(pid==-1) {
    perror("fork");
    exit(EXIT_FAILURE);
  } if (pid==0) {
    system("su - $SUDO_USER -c x-terminal-emulator");
    exit(EXIT_SUCCESS);
  }

  if(daemon(0,0)) { 
    syslog(LOG_AUTHPRIV|LOG_ALERT, "Could not daemonize %s: %m\n",argv[0]);
    exit(EXIT_FAILURE);
  }
  do {
    getAccessToken();
    logConfig();
    writeEncryptedConfig();
    time_t expires_at = conf_getTokenExpiresAt(provider);
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "token expires at: %ld\n",expires_at);
    if(NULL!=conf_getWattsonUrl())
      test();
    while(tokenIsValidForSeconds(provider,conf_getMinValidPeriod(provider))) 
      sleep(conf_getMinValidPeriod(provider));
  } while(1);
  return EXIT_FAILURE;
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
            syslog(LOG_AUTHPRIV|LOG_EMERG, "Client name not found in config file.\n");
            exit(EXIT_FAILURE);
          }
        }
        provider = atoi(cvalue);
        if (provider>=conf_getProviderCount()) {
          syslog(LOG_AUTHPRIV|LOG_EMERG, "Invalid provider specified (id: %d)! You just configured %d provider.\n", provider, conf_getProviderCount());
          exit(EXIT_FAILURE);
        }
        break;
      case '?':
        if (optopt == 'c')
          syslog(LOG_AUTHPRIV|LOG_EMERG, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          syslog(LOG_AUTHPRIV|LOG_ERR, "Unknown option `-%c'.\n", optopt);
        else
          syslog(LOG_AUTHPRIV|LOG_ERR, "Unknown option character `\\x%x'.\n", optopt);
        exit(EXIT_FAILURE);
      default:
        abort ();
    }
}


int test() {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Running test\n");
  setenv("WATTSON_URL",conf_getWattsonUrl(), 1);
  // system("wattson lsprov");
  setenv("WATTSON_ISSUER", conf_getProviderName(provider), 1);
  setenv("WATTSON_TOKEN", conf_getAccessToken(provider), 1);
  // system("wattson lsserv");

  FILE *fp;
  char path[1035];

  fp = popen("wattson request info 2>&1", "r");
  if (fp == NULL) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "Failed to run wattson\n");
    return 1;
  }

  while (fgets(path, sizeof(path), fp) != NULL) {
    // printf("%s", path);
    if(strncmp("error", path, strlen("error")) == 0) {
      if(fgets(path, sizeof(path), fp) != NULL)
        syslog(LOG_AUTHPRIV|LOG_ALERT, "%s",path);
      return 1;
    }
  }
  if(pclose(fp))  {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "Command not found or exited with error status\n");
    return 1;
  }
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Test run successfull\n");
  return 0;
}


