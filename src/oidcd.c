#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <syslog.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>

#include "oidcd.h"
#include "config.h"
#include "oidc.h"
#include "token_ipc.h"

void sig_handler(int signo) {
  switch(signo) {
    case SIGSEGV:
      syslog(LOG_AUTHPRIV|LOG_EMERG, "Caught Signal SIGSEGV");
      break;
    default: 
      syslog(LOG_AUTHPRIV|LOG_EMERG, "Caught Signal %d", signo);
  }
  exit(signo);
}

int main(int argc, char** argv) {
  openlog("oidc-service", LOG_CONS|LOG_PID, LOG_AUTHPRIV);
  setlogmask(LOG_UPTO(LOG_DEBUG));
  // setlogmask(LOG_UPTO(LOG_NOTICE));
  readSavedConfig();
  readConfig();
  signal(SIGSEGV, sig_handler);
  if(daemon(0,0)) { 
    syslog(LOG_AUTHPRIV|LOG_ALERT, "Could not daemonize %s: %m\n",argv[0]);
    exit(EXIT_FAILURE);
  }
  pthread_t ipc;
  if(pthread_create(&ipc, NULL, communicate, NULL)) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "could not create Thread. %m");
    exit(EXIT_FAILURE);
  }
  if(pthread_detach(ipc)) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "Could not detach Thread. %m");
    exit(EXIT_FAILURE);
  }
  do {
    unsigned int provider;
    for(provider=0; provider<conf_getProviderCount(); provider++) {
      if(getAccessToken(provider)!=0) {
        return EXIT_FAILURE;
      }
      logConfig();
      time_t expires_at = conf_getTokenExpiresAt(provider);
      syslog(LOG_AUTHPRIV|LOG_DEBUG, "token expires at: %ld\n",expires_at);
      saveConfig();
      if(NULL!=conf_getWattsonUrl())
        test(provider);
    }
    sort_provider(); // sorts provider by the time until token has to be refreshed
    while(tokenIsValidForSeconds(0,conf_getMinValidPeriod(0))) 
      sleep(conf_getTokenExpiresAt(0)-time(NULL)-conf_getMinValidPeriod(0));
  } while(1);
  return EXIT_FAILURE;
}


int test(int provider) {
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

