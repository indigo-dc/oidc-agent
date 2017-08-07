#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <time.h>

#include "oidcd.h"
#include "config.h"
#include "oidc.h"
#include "api.h"

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

void daemonize() {
  pid_t pid;
  if ((pid = fork ()) != 0)
    exit (EXIT_FAILURE);
  if (setsid() < 0) {
    exit (EXIT_FAILURE);
  }
  signal(SIGHUP, SIG_IGN);
  if ((pid = fork ()) != 0)
    exit (EXIT_FAILURE);
  chdir ("/");
  umask (0);
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
  open("/dev/null", O_RDONLY);
  open("/dev/null", O_RDWR);
  open("/dev/null", O_RDWR);
}

int main(int argc, char** argv) {
  // openlog("oidc-service", LOG_CONS|LOG_PID, LOG_AUTHPRIV);
  // setlogmask(LOG_UPTO(LOG_DEBUG));
  // // setlogmask(LOG_UPTO(LOG_NOTICE));
  // signal(SIGSEGV, sig_handler);
  // savecwd();  // cwd has to be set before calling daemonize
  // daemonize();
  // readSavedConfig();
  // readConfig();
  // struct connection* con = initTokenSocket(); // can't call it directly after daemonizing, because the child might not have exited yet and therefore the ppid could be  wrong
  // do {
  //   unsigned int provider;
  //   for(provider=0; provider<conf_getProviderCount(); provider++) {
  //     if(getAccessToken(provider)!=0) {
  //       return EXIT_FAILURE;
  //     }
  //     logConfig();
  //     time_t expires_at = conf_getTokenExpiresAt(provider);
  //     syslog(LOG_AUTHPRIV|LOG_DEBUG, "token expires at: %ld\n",expires_at);
  //     saveConfig();
  //     if(NULL!=conf_getWattsonUrl())
  //       test(provider);
  //   }
  //   sort_provider(); // sorts provider by the time until token has to be refreshed
  //   while(tokenIsValidForSeconds(0,conf_getMinValidPeriod(0)))  {
  //     // sleep(conf_getTokenExpiresAt(0)-time(NULL)-conf_getMinValidPeriod(0));
  //     if (tryAccept(con, tokenIsValidForSeconds(0,conf_getMinValidPeriod(0)))>0) {
  //       //accepted a connection so we can communicate
  //       communicate(con); 
  //     }
  //   }
  // } while(1);
  return EXIT_FAILURE;
}



