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
#include "file_io.h"
#include "ipc.h"

#define TOKEN_FILE "/access_token"
#define ENV_VAR "OIDC_TOKEN"

int main(int argc, char** argv) {
  openlog("oidc-service", LOG_CONS|LOG_PID|LOG_PERROR, LOG_AUTHPRIV);
  setlogmask(LOG_UPTO(LOG_DEBUG));
  // setlogmask(LOG_UPTO(LOG_NOTICE));
  readConfig();
  parseOpt(argc, argv);
  cwd = getcwd(NULL,0);
  if(cwd==NULL) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "Could not get cwd: %m\n");
    exit(EXIT_FAILURE);
  }
  int pid = fork();
  if(pid==-1) {
    perror("fork");
    exit(EXIT_FAILURE);
  } if (pid==0) {
    system("x-terminal-emulator");
    exit(EXIT_SUCCESS);
  }

  if(daemon(0,0)) { 
    syslog(LOG_AUTHPRIV|LOG_ALERT, "Could not daemonize %s: %m\n",argv[0]);
    exit(EXIT_FAILURE);
  }
  do {
    getAccessToken();
    logConfig();
    time_t expires_at = time(NULL)+conf_getTokenExpiresIn(provider);
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "token_expires_in: %lu\n",conf_getTokenExpiresIn(provider));
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "token expires at: %ld\n",expires_at);
    if(conf_getTokenExpiresIn(provider)<=0)
      break;
    test();
    while(expires_at-time(NULL)>conf_getMinValidPeriod(provider)) 
      sleep(conf_getMinValidPeriod(provider));
  } while(1);
  return EXIT_FAILURE;
}

int getAccessToken() {
  if (conf_getRefreshToken(provider)!=NULL && strcmp("", conf_getRefreshToken(provider))!=0) 
    if(tryRefreshFlow()==0)
      return 0;
  syslog(LOG_AUTHPRIV|LOG_NOTICE, "No valid refresh_token found for this client.\n");
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

int tryRefreshFlow() {
  refreshFlow(0);
  if(NULL==conf_getAccessToken(provider))
    return 1;
#ifdef TOKEN_FILE
  writeToFile(TOKEN_FILE, conf_getAccessToken(provider));
#endif
#ifdef ENV_VAR
  setenv(ENV_VAR, conf_getAccessToken(provider),1);
#endif
  return 0;
}

int tryPasswordFlow() {
  if(conf_getUsername(provider)==NULL || strcmp("", conf_getUsername(provider))==0)
    conf_setUsername(provider, getUserInput("No username specified. Enter username for client: "));
  char* password = getUserInput("Enter password for client: ");
  if(passwordFlow(provider, password)!=0 || NULL==conf_getAccessToken(provider)) {
    free(password);
    syslog(LOG_AUTHPRIV|LOG_EMERG, "Could not get an access_token\n");
    return 1;
  }
  free(password);
#ifdef TOKEN_FILE
  writeToFile(TOKEN_FILE, conf_getAccessToken(provider));
#endif
#ifdef ENV_VAR
  setenv(ENV_VAR, conf_getAccessToken(provider),1);
#endif
  return 0;
}


int test() {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Running test\n");
  setenv("WATTSON_URL","https://watts-dev.data.kit.edu" ,0);
  // system("wattson lsprov");
  setenv("WATTSON_ISSUER", conf_getProviderName(provider),1);
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
        fprintf(stderr, "%s",path);
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

void runPassprompt() {
  int pid = fork();
  if(pid==-1) {
    perror("fork");
    exit(EXIT_FAILURE);
  }
  if (pid==0) {
    char* fmt = "x-terminal-emulator -e %s/oidc-prompt/bin/oidc-prompt";
    char* cmd = calloc(sizeof(char),strlen(fmt)+strlen(cwd)-2+1);
    sprintf(cmd, fmt, cwd);
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "running callback cmd: %s\n",cmd);
    system(cmd);
    free(cmd);
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "exiting callback\n");
    exit(EXIT_SUCCESS);
  }
}

char* getUserInput(char* prompt) {
  ipc_close();
  ipc_init();
  int msgsock = ipc_bind(runPassprompt);
  if(ipc_write(msgsock, prompt)!=0) 
    return NULL;
  char* password = ipc_read(msgsock);
  ipc_close();
  return password;
}
