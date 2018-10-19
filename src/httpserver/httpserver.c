#define _GNU_SOURCE
#include "httpserver.h"
#include "../ipc/ipc.h"
#include "../utils/memory.h"
#include "../utils/portUtils.h"
#include "../utils/stringUtils.h"
#include "requestHandler.h"
#include "running_server.h"

#include "../../lib/list/src/list.h"

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <syslog.h>
#include <unistd.h>

void stopHttpServer(struct MHD_Daemon** d_ptr) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "HttpServer: Stopping HttpServer");
  MHD_stop_daemon(*d_ptr);
  secFree(d_ptr);
}

/**
 * @param config a pointer to a json account config.
 * */
struct MHD_Daemon** startHttpServer(const char* redirect_uri, char* config,
                                    char* state) {
  struct MHD_Daemon** d_ptr = secAlloc(sizeof(struct MHD_Daemon*));
  char**              cls   = secAlloc(sizeof(char*) * 3);
  cls[0]                    = oidc_strcopy(config);
  cls[1]                    = oidc_strcopy(redirect_uri);
  cls[2]                    = oidc_strcopy(state);
  unsigned short port       = getPortFromUri(redirect_uri);
  *d_ptr = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION, port, NULL, NULL,
                            &request_echo, cls, MHD_OPTION_END);

  if (*d_ptr == NULL) {
    syslog(LOG_AUTHPRIV | LOG_ERR, "Error starting the HttpServer on port %d",
           port);
    oidc_errno = OIDC_EHTTPD;
    secFree(d_ptr);
    secFreeArray(cls, 3);
    return NULL;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "HttpServer: Started HttpServer on port %d",
         port);
  return d_ptr;
}

struct MHD_Daemon** d_ptr   = NULL;
list_t*             servers = NULL;

void http_sig_handler(int signo) {
  switch (signo) {
    case SIGTERM:
      sleep(5);
      stopHttpServer(d_ptr);
      break;
    default:
      syslog(LOG_AUTHPRIV | LOG_EMERG, "HttpServer caught Signal %d", signo);
  }
  exit(signo);
}

oidc_error_t fireHttpServer(list_t* redirect_uris, size_t size, char* config,
                            char* state) {
  int fd[2];
  if (pipe2(fd, O_DIRECT) != 0) {
    oidc_setErrnoError();
    return oidc_errno;
  }
  pid_t pid = fork();
  if (pid == -1) {
    syslog(LOG_AUTHPRIV | LOG_ALERT, "fork %m");
    oidc_setErrnoError();
    return oidc_errno;
  }
  if (pid == 0) {
    prctl(PR_SET_PDEATHSIG, SIGTERM);
    close(fd[0]);
    size_t i;
    for (i = 0; i < size && d_ptr == NULL; i++) {
      d_ptr = startHttpServer(list_at(redirect_uris, i)->val, config, state);
    }
    if (d_ptr == NULL) {
      ipc_write(fd[1], "%d", OIDC_EHTTPPORTS);
      close(fd[1]);
      exit(EXIT_FAILURE);
    }
    ipc_write(fd[1], "%hu", getPortFromUri(list_at(redirect_uris, i - 1)->val));
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
    if (e == NULL) {
      return oidc_errno;
    }
    char**   endptr = secAlloc(sizeof(char*));
    long int port   = strtol(e, endptr, 10);
    if (**endptr != '\0') {
      secFree(endptr);
      secFree(e);
      oidc_errno = OIDC_EERROR;
      oidc_seterror("Internal error. Could not convert pipe communication.");
      return oidc_errno;
    }
    secFree(endptr);
    secFree(e);
    if (port < 0) {
      oidc_errno = port;
      return oidc_errno;
    }
    if (servers == NULL) {
      servers        = list_new();
      servers->free  = (void (*)(void*)) & secFreeRunningServer;
      servers->match = (int (*)(void*, void*)) & matchRunningServer;
    }
    struct running_server* running_server =
        secAlloc(sizeof(struct running_server));
    running_server->pid   = pid;
    running_server->state = oidc_strcopy(state);
    list_rpush(servers, list_node_new(running_server));

    return port;
  }
}

void termHttpServer(const char* state) {
  if (state == NULL) {
    return;
  }
  if (servers == NULL) {
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "No servers running");
    return;
  }
  list_node_t* n = list_find(servers, (char*)state);
  if (n == NULL) {
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "No server found for state %s", state);
    return;
  }
  kill(((struct running_server*)n->val)->pid, SIGTERM);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "killed webserver for state %s with pid %d",
         state, ((struct running_server*)n->val)->pid);
  list_remove(servers, n);
}
