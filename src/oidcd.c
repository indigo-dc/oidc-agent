#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <time.h>
#include <getopt.h>
#include <ctype.h>

#include "oidcd.h"
#include "oidc.h"
#include "file_io.h"
#include "ipc.h"
#include "provider.h"
#include "oidc_string.h"

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
  if ((pid = fork ()) ==-1) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "fork %m");
    exit(EXIT_FAILURE);
  } else if (pid > 0) {
    exit (EXIT_SUCCESS);
  }
  if (setsid() < 0) {
    exit (EXIT_FAILURE);
  }
  signal(SIGHUP, SIG_IGN);
  if ((pid = fork ()) ==-1) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "fork %m");
    exit(EXIT_FAILURE);
  } else if (pid > 0) {
    printf("%s=%d; export %s;\n", OIDC_PID_ENV_NAME, pid, OIDC_PID_ENV_NAME);
    printf("echo Daemon pid $%s\n", OIDC_PID_ENV_NAME);
    exit (EXIT_SUCCESS);
  }
  chdir ("/");
  umask (0);
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
  open("/dev/null", O_RDONLY);
  open("/dev/null", O_RDWR);
  open("/dev/null", O_RDWR);
}

void handleGen(char* q, int sock, struct oidc_provider** loaded_p, size_t* loaded_p_count) {
  char* provider_json = q+strlen("gen:");
  struct oidc_provider* provider = getProviderFromJSON(provider_json);
  if(provider!=NULL) {
    if(getAccessToken(provider, FORCE_NEW_TOKEN)!=0) {
      ipc_write(sock, RESPONSE_ERROR, "misconfiguration or network issues"); 
      freeProvider(provider);
      return;
    } 
    if(isValid(provider_getRefreshToken(*provider))) {
      ipc_write(sock, RESPONSE_STATUS_REFRESH, "success", provider_getRefreshToken(*provider));
    } else {
      ipc_write(sock, RESPONSE_STATUS, "success");
    }
    *loaded_p = addProvider(*loaded_p, loaded_p_count, *provider);
    free(provider);
  } else {
    ipc_write(sock, RESPONSE_ERROR, "json malformed");
  }
}

void handleAdd(char* q, int sock, struct oidc_provider** loaded_p, size_t* loaded_p_count) {
  char* provider_json = q+strlen("add:");
  struct oidc_provider* provider = getProviderFromJSON(provider_json);
  if(provider!=NULL) {
    if(NULL!=findProvider(*loaded_p, *loaded_p_count, *provider)){
      freeProvider(provider);
      ipc_write(sock, RESPONSE_ERROR, "provider already loaded");
      return;
    }
    if(getAccessToken(provider, FORCE_NEW_TOKEN)!=0) {
      freeProvider(provider);
      ipc_write(sock, RESPONSE_ERROR, "misconfiguration or network issues"); 
      return;
    } 
    *loaded_p = addProvider(*loaded_p, loaded_p_count, *provider);
    free(provider);
    ipc_write(sock, RESPONSE_STATUS, "success");
  } else {
    ipc_write(sock, RESPONSE_ERROR, "json malformed");
  }
}

void handleClient(char* q, int sock, struct oidc_provider** loaded_p, size_t* loaded_p_count) {
  struct key_value pairs[3];
  pairs[0].key = "request";
  pairs[1].key = "provider";
  pairs[2].key = "min_valid_period";
  if(getJSONValues(q+strlen("client:"), pairs, sizeof(pairs)/sizeof(*pairs))<0) {
    ipc_write(sock, RESPONSE_ERROR, "json malformed");
    free(pairs[0].value); free(pairs[1].value); free(pairs[2].value);
    return;
  }
  char* request = pairs[0].value;
  if(request==NULL) {
    ipc_write(sock, RESPONSE_ERROR, "json malformed");
    free(pairs[0].value); free(pairs[1].value); free(pairs[2].value);
    return;
  }
  if(strcmp(request, "provider_list")==0) {
    char* providerList = getProviderNameList(*loaded_p, *loaded_p_count);
    ipc_write(sock, RESPONSE_STATUS_PROVIDER, "success", providerList);
    free(providerList);
  } else if(strcmp(request, "access_token")==0) {
    if(pairs[1].value==NULL || pairs[2].value == NULL) {
      ipc_write(sock, RESPONSE_ERROR, "Bad request. Need provider name and min_valid_period for getting access token.");
      free(pairs[0].value); free(pairs[1].value); free(pairs[2].value);
      return;
    }
    struct oidc_provider key = {0, pairs[1].value, 0, 0, 0, 0, 0, 0, 0, {0, 0}};
    time_t min_valid_period = atoi(pairs[2].value);
    struct oidc_provider* provider = findProvider(*loaded_p, *loaded_p_count, key);
    if(provider==NULL) {
      ipc_write(sock, RESPONSE_ERROR, "Provider %s not loaded.", pairs[1].value);
      free(pairs[0].value); free(pairs[1].value); free(pairs[2].value);
      return;
    }
    if(getAccessToken(provider, min_valid_period)!=0) {
      ipc_write(sock, RESPONSE_ERROR, "Could not get access token.");
      free(pairs[0].value); free(pairs[1].value); free(pairs[2].value);
      return;
    }
    ipc_write(sock, RESPONSE_STATUS_ACCESS, "success", provider_getAccessToken(*provider));
  } else {
    ipc_write(sock, RESPONSE_ERROR, "Bad request");
  }
    free(pairs[0].value); free(pairs[1].value); free(pairs[2].value);
}

int main(int argc, char** argv) {
  openlog("oidcd", LOG_CONS|LOG_PID, LOG_AUTHPRIV);
  // setlogmask(LOG_UPTO(LOG_DEBUG));
  setlogmask(LOG_UPTO(LOG_NOTICE));

  char* pidstr;
  pid_t pid;
  int c;
  while ((c = getopt (argc, argv, "k")) != -1)
    switch (c) {
      case 'k':
        pidstr = getenv(OIDC_PID_ENV_NAME);
        if(pidstr == NULL) {
          fprintf(stderr, "%s not set, cannot kill daemon\n", OIDC_PID_ENV_NAME);
          exit(EXIT_FAILURE);
        }
        pid = atoi(pidstr);
        if(0 == pid) {
          fprintf(stderr, "%s not set to a valid pid: %s\n", OIDC_PID_ENV_NAME, pidstr);
          exit(EXIT_FAILURE);
        }
        if (kill(pid, SIGTERM) == -1) {
          perror("kill");
          exit(EXIT_FAILURE);
        } else {
          printf("unset %s;\n", OIDC_SOCK_ENV_NAME);
          printf("unset %s;\n", OIDC_PID_ENV_NAME);
          printf("echo Daemon pid %d killed;\n", pid);
          exit(EXIT_SUCCESS);
        }
        break;
      case '?':
        if (isprint (optopt))
          printf("Unknown option `-%c'.\n", optopt);
        else
          printf("Unknown option character `\\x%x'.\n", optopt);
        exit(EXIT_FAILURE);
      default:
        abort ();
    }


  signal(SIGSEGV, sig_handler);

  char* oidc_dir = getOidcDir();
  if (NULL==oidc_dir) {
    printf("Could not find an oidc directory. Please make one. I might do it myself in a future version.\n");
    exit(EXIT_FAILURE);
  }
  free(oidc_dir);

  struct connection* listencon = calloc(sizeof(struct connection), 1);
  ipc_init(listencon, "gen", OIDC_SOCK_ENV_NAME, 1);
  daemonize();

  ipc_bindAndListen(listencon);

  struct oidc_provider* loaded_p = NULL;
  struct oidc_provider** loaded_p_addr = &loaded_p;
  size_t loaded_p_count = 0;

  struct connection* clientcons = NULL;
  struct connection** clientcons_addr = &clientcons;
  size_t number_clients = 0;

  while(1) {
    struct connection* con = ipc_async(*listencon, clientcons_addr, &number_clients);
    if(con==NULL) {
      //TODO should not happen
    } else {
      char* q = ipc_read(*(con->msgsock));
      if(NULL!=q) {
        if(strstarts(q, "gen:")) {
          handleGen(q, *(con->msgsock), loaded_p_addr, &loaded_p_count);
        } else if(strstarts(q, "add:")) {
          handleAdd(q, *(con->msgsock), loaded_p_addr, &loaded_p_count);
        } else if(strstarts(q, "client:")) {
          handleClient(q, *(con->msgsock), loaded_p_addr, &loaded_p_count);
        } else {
          ipc_write(*(con->msgsock), RESPONSE_ERROR, "Bad request");
        }
        free(q);
      }
      syslog(LOG_AUTHPRIV|LOG_DEBUG, "Remove con from pool");
      clientcons = removeConnection(*clientcons_addr, &number_clients, con);
      clientcons_addr = &clientcons;
    }
  }
  return EXIT_FAILURE;
}



