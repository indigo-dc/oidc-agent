#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "../../src/ipc.h"
#include "prompt.h"

static struct connection con;

void sig_handler(int signo) {
  switch(signo) {
    case SIGINT:
      syslog(LOG_AUTHPRIV|LOG_DEBUG, "Received Signal SIGINT");
      break;
    case SIGHUP:
      syslog(LOG_AUTHPRIV|LOG_DEBUG, "Received Signal SIGHUP");
      break;
    default: 
      syslog(LOG_AUTHPRIV|LOG_EMERG, "Caught Signal %d", signo);
  }
  ipc_closeAndUnlink(&con);
  exit(signo);
}

int main(/* int argc,char** argv */) {
  openlog("oidc-prompt", LOG_CONS|LOG_PID, LOG_AUTHPRIV);
  setlogmask(LOG_UPTO(LOG_DEBUG));
  // setlogmask(LOG_UPTO(LOG_NOTICE));
  ipc_init(&con, "prompt", "OIDC_PROMPT_SOCKET_PATH", 0);
  signal(SIGINT, sig_handler);
  signal(SIGHUP, sig_handler);
  int sock = ipc_connect(con);
  if(sock<0) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "Could not connect to socket '%s'", con.server->sun_path);
    exit(EXIT_FAILURE);
  }
  while(1) {
    char* prompt_str = ipc_read(sock);
    if(prompt_str==NULL) {
      ipc_closeAndUnlink(&con);
      return EXIT_FAILURE;
    }
    char* res;
    switch(*prompt_str) {
      case PRINT_CHAR:
        printf("%s\n",prompt_str+1);
        break;
      case PRINT_AND_CLOSE_CHAR:
        printf("%s\n", prompt_str+1);
        free(prompt_str);
        ipc_closeAndUnlink(&con);
        printf("Press any key to exit...");
        getchar();
        return EXIT_SUCCESS;
      case PROMPT_CHAR:
        res = prompt(prompt_str+1);
        if(ipc_write(sock, res)!=0) {
          ipc_closeAndUnlink(&con);
          return EXIT_FAILURE;
        }
        memset(res, 0, strlen(res));
        free(res);
        break;
      case PROMPT_PASSWORD_CHAR:
        res = promptPassword(prompt_str+1);
        if(ipc_write(sock, res)!=0){
          ipc_closeAndUnlink(&con);
          return EXIT_FAILURE;
        }
        memset(res, 0, strlen(res));
        free(res);
        break;
      default:
        syslog(LOG_AUTHPRIV|LOG_ALERT, "IPC Read malformed. Unknown mode %d %c.", *prompt_str, *prompt_str);
        free(prompt_str);
        ipc_closeAndUnlink(&con);
        return EXIT_FAILURE;
    }
    free(prompt_str);
  }
  ipc_closeAndUnlink(&con);
}
