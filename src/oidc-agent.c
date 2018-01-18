#define _XOPEN_SOURCE 500

#include "oidc-agent.h"
#include "ipc.h"
#include "account.h"
#include "settings.h"
#include "oidc_error.h"
#include "agent_handler.h"

#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <libgen.h>
#include <sys/stat.h>

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
  if((pid = fork()) == -1) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "fork %m");
    exit(EXIT_FAILURE);
  } else if(pid > 0) {
    exit(EXIT_SUCCESS);
  }
  if(setsid() < 0) {
    exit(EXIT_FAILURE);
  }
  signal(SIGHUP, SIG_IGN);
  if((pid = fork()) == -1) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "fork %m");
    exit(EXIT_FAILURE);
  } else if(pid > 0) {
    printf("%s=%d; export %s;\n", OIDC_PID_ENV_NAME, pid, OIDC_PID_ENV_NAME);
    printf("echo Agent pid $%s\n", OIDC_PID_ENV_NAME);
    exit(EXIT_SUCCESS);
  }
  chdir("/");
  umask(0);
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
  open("/dev/null", O_RDONLY);
  open("/dev/null", O_RDWR);
  open("/dev/null", O_RDWR);
}


int main(int argc, char** argv) {
  openlog("oidc-agent", LOG_CONS|LOG_PID, LOG_AUTHPRIV);
  setlogmask(LOG_UPTO(LOG_NOTICE));
  struct arguments arguments;

  /* Set argument defaults */
  arguments.kill_flag = 0;
  arguments.console = 0;
  arguments.debug = 0;
  srandom(time(NULL));

  argp_parse (&argp, argc, argv, 0, 0, &arguments);
  if(arguments.debug) {
    setlogmask(LOG_UPTO(LOG_DEBUG));
  }

  if(arguments.kill_flag) {
    char* pidstr = getenv(OIDC_PID_ENV_NAME);
    if(pidstr == NULL) {
      fprintf(stderr, "%s not set, cannot kill Agent\n", OIDC_PID_ENV_NAME);
      exit(EXIT_FAILURE);
    }
    pid_t pid = atoi(pidstr);
    if(0 == pid) {
      fprintf(stderr, "%s not set to a valid pid: %s\n", OIDC_PID_ENV_NAME, pidstr);
      exit(EXIT_FAILURE);
    }
    if (kill(pid, SIGTERM) == -1) {
      perror("kill");
      exit(EXIT_FAILURE);
    } else {
      unlink(getenv(OIDC_SOCK_ENV_NAME));
      rmdir(dirname(getenv(OIDC_SOCK_ENV_NAME)));
      printf("unset %s;\n", OIDC_SOCK_ENV_NAME);
      printf("unset %s;\n", OIDC_PID_ENV_NAME);
      printf("echo Agent pid %d killed;\n", pid);
      exit(EXIT_SUCCESS);
    }
  }

  // signal(SIGSEGV, sig_handler);

  struct connection* listencon = calloc(sizeof(struct connection), 1);
  if(ipc_init(listencon, OIDC_SOCK_ENV_NAME, 1)!=OIDC_SUCCESS) {
    fprintf(stderr, "%s\n", oidc_serror());
    exit(EXIT_FAILURE);
  }
  if(!arguments.console) {
    daemonize();
  }

  ipc_bindAndListen(listencon);

  struct oidc_account* loaded_p = NULL;
  struct oidc_account** loaded_p_addr = &loaded_p;
  size_t loaded_p_count = 0;

  struct connection* clientcons = NULL;
  struct connection** clientcons_addr = &clientcons;
  size_t number_clients = 0;

  while(1) {
    struct connection* con = ipc_async(*listencon, clientcons_addr, &number_clients);
    if(con==NULL) {
      // should never happen
      syslog(LOG_AUTHPRIV|LOG_ALERT, "Something went wrong");
    } else {
      char* q = ipc_read(*(con->msgsock));
      if(NULL!=q) {
        struct key_value pairs[8];
        pairs[0].key = "request"; pairs[0].value = NULL;
        pairs[1].key = "account"; pairs[1].value = NULL;
        pairs[2].key = "min_valid_period"; pairs[2].value = NULL;
        pairs[3].key = "config"; pairs[3].value = NULL;
        pairs[4].key = "flow"; pairs[4].value = NULL;
        pairs[5].key = "code"; pairs[5].value = NULL;
        pairs[6].key = "redirect_uri"; pairs[6].value = NULL;
        pairs[7].key = "state"; pairs[7].value = NULL;
        if(getJSONValues(q, pairs, sizeof(pairs)/sizeof(*pairs))<0) {
          ipc_write(*(con->msgsock), "Bad request: %s", oidc_serror());
        } else {
          if(pairs[0].value) {
            if(strcmp(pairs[0].value, REQUEST_VALUE_GEN)==0) {
              agent_handleGen(*(con->msgsock), loaded_p_addr, &loaded_p_count, pairs[3].value, pairs[4].value);
            } else if(strcmp(pairs[0].value, REQUEST_VALUE_CODEEXCHANGE)==0 ) {
              agent_handleCodeExchange(*(con->msgsock), loaded_p_addr, &loaded_p_count, pairs[3].value, pairs[5].value, pairs[6].value, pairs[7].value);
            } else if(strcmp(pairs[0].value, REQUEST_VALUE_STATELOOKUP)==0 ) {
              agent_handleStateLookUp(*(con->msgsock), *loaded_p_addr, loaded_p_count, pairs[7].value);
            } else if(strcmp(pairs[0].value, REQUEST_VALUE_ADD)==0) {
              agent_handleAdd(*(con->msgsock), loaded_p_addr, &loaded_p_count, pairs[3].value);
            } else if(strcmp(pairs[0].value, REQUEST_VALUE_REMOVE)==0) {
              agent_handleRm(*(con->msgsock), loaded_p_addr, &loaded_p_count, pairs[3].value, 0);
            } else if(strcmp(pairs[0].value, REQUEST_VALUE_DELETE)==0) {
              agent_handleRm(*(con->msgsock), loaded_p_addr, &loaded_p_count, pairs[3].value, 1);
            } else if(strcmp(pairs[0].value, REQUEST_VALUE_ACCESSTOKEN)==0) {
              agent_handleToken(*(con->msgsock), *loaded_p_addr, loaded_p_count, pairs[1].value, pairs[2].value);
            } else if(strcmp(pairs[0].value, REQUEST_VALUE_ACCOUNTLIST)==0) {
              agent_handleList(*(con->msgsock), *loaded_p_addr, loaded_p_count);
            } else if(strcmp(pairs[0].value, REQUEST_VALUE_REGISTER)==0) {
              agent_handleRegister(*(con->msgsock), *loaded_p_addr, loaded_p_count, pairs[3].value);
            } else {
              ipc_write(*(con->msgsock), "Bad request. Unknown request type.");
            }
          } else {
            ipc_write(*(con->msgsock), "Bad request. No request type.");
          }
        }
        clearFreeKeyValuePairs(pairs, sizeof(pairs)/sizeof(*pairs));
        clearFreeString(q);
      }
      syslog(LOG_AUTHPRIV|LOG_DEBUG, "Remove con from pool");
      clientcons = removeConnection(*clientcons_addr, &number_clients, con);
      clientcons_addr = &clientcons;
    }
  }
  return EXIT_FAILURE;
}

