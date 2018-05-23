#define _GNU_SOURCE
#include "httpserver.h"
#include "../ipc.h"
#include "requestHandler.h"
#include "running_server.h"
#include "../free/cleaner.h"
#include "../oidc_utilities.h"

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <syslog.h>
#include <signal.h>
#include <unistd.h>
#include <sys/prctl.h>


void stopHttpServer(struct MHD_Daemon** d_ptr) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "HttpServer: Stopping HttpServer");
  MHD_stop_daemon(*d_ptr);
  clearFree(d_ptr, sizeof(struct MHD_Daemon*));
}

/**
 * @param config a pointer to a json account config.
 * */
struct MHD_Daemon** startHttpServer(unsigned short port, char* config, char* state) {
  struct MHD_Daemon** d_ptr = calloc(sizeof(struct MHD_Daemon*),1);
  char** cls = calloc(sizeof(char*), 3);
  cls[0] = oidc_sprintf("%s", config);
  cls[1] = portToUri(port);
  cls[2] = oidc_sprintf("%s", state);
  *d_ptr = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
      port,
      NULL,
      NULL,
      &request_echo,
      cls,
      MHD_OPTION_END);

  if(*d_ptr == NULL) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "Error starting the HttpServer on port %d", port);
    oidc_errno = OIDC_EHTTPD;
    clearFree(d_ptr, sizeof(struct MHD_Daemon*));
    clearFreeStringArray(cls, 3);
    return NULL;
  }
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "HttpServer: Started HttpServer on port %d", port);
  return d_ptr;
}


struct MHD_Daemon** d_ptr = NULL;
list_t* servers = NULL;


void http_sig_handler(int signo) {
  switch(signo) {
    case SIGTERM:
      sleep(5);
      stopHttpServer(d_ptr);
      break;
    default: 
      syslog(LOG_AUTHPRIV|LOG_EMERG, "HttpServer caught Signal %d", signo);
  }
  exit(signo);
}


oidc_error_t fireHttpServer(unsigned short* port, size_t size, char* config, char* state) {
  int fd[2];
  if(pipe2(fd, O_DIRECT)!=0) {
    oidc_setErrnoError();
    return oidc_errno;
  }
  pid_t pid = fork();
  if(pid == -1) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "fork %m");
    oidc_setErrnoError();
    return oidc_errno;
  }
  if(pid == 0) {
    prctl(PR_SET_PDEATHSIG, SIGTERM);
    close(fd[0]);
    size_t i;
    for(i=0; i<size && d_ptr==NULL; i++) {
      d_ptr = startHttpServer(port[i], config, state);
    }
    if(d_ptr==NULL) {
      ipc_write(fd[1], "%d", OIDC_EHTTPPORTS);
      close(fd[1]);
      exit(EXIT_FAILURE);
    }
    ipc_write(fd[1], "%hu", port[i-1]);
    close(fd[1]);
    signal(SIGTERM, http_sig_handler);
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGTERM);
    sigsuspend(&sigset); 
    return OIDC_ENOPE;
  } else {
    close(fd[1]);
    char* e = ipc_read(fd[0]);
    if(e==NULL) {
      return oidc_errno;
    }
    char** endptr = calloc(sizeof(char*), 1);
    long int port = strtol(e, endptr, 10);
    if(**endptr!='\0') {
      clearFree(endptr, sizeof(char*));
      clearFreeString(e);
      oidc_errno = OIDC_EERROR;
      oidc_seterror("Internal error. Could not convert pipe communication.");
      return oidc_errno;
    }
    clearFree(endptr, sizeof(char*));
    clearFreeString(e);
    if(port<0) {
      oidc_errno = port;
      return oidc_errno;
    }
    if(servers == NULL) {
      servers = list_new();
      servers->free = (void(*) (void*)) &clearFreeRunningServer;
      servers->match = (int(*) (void*, void*)) &matchRunningServer;
    }
    struct running_server*  running_server = calloc(sizeof(struct running_server), 1);
    running_server->pid = pid;
    running_server->state = oidc_strcopy(state);
    list_rpush(servers, list_node_new(running_server));

    return port;
  }
}

void termHttpServer(char* state) {
  if(state==NULL) {
    return;
  }
  if(servers==NULL) {
    return;
  }
  list_node_t* n = list_find(servers, state);  
  if(n==NULL) {
    return;
  }
  kill(((struct running_server*)n->val)->pid, SIGTERM);
  list_remove(servers, n);
}
