#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <stdio.h>
#include <unistd.h>

#include "../../src/ipc.h"
#include "prompt.h"

int main(/* int argc,char** argv */) {
  openlog("oidc-prompt", LOG_CONS|LOG_PID, LOG_AUTHPRIV);
  // setlogmask(LOG_UPTO(LOG_DEBUG));
  setlogmask(LOG_UPTO(LOG_NOTICE));
  static struct connection con;
  ipc_init(&con, "prompt", "OIDC_PROMPT_SOCKET_PATH", 0);
  int sock = ipc_connect(con);
  while(1) {
    char* prompt_str = ipc_read(sock);
    if(prompt_str==NULL) {
      return EXIT_FAILURE;
    }
    char* res;
    switch(*prompt_str) {
      case PRINT_CHAR:
        printf("%s\n",prompt_str+1);
        break;
      case PRINT_AND_CLOSE_CHAR:
        printf("%s\n", prompt_str+1);
        printf("Press any key to exit...");
        free(prompt_str);
        getchar();
        return EXIT_SUCCESS;
      case PROMPT_CHAR:
        res = prompt(prompt_str+1);
        if(ipc_write(sock, res)!=0)
          return EXIT_FAILURE;
        memset(res, 0, strlen(res));
        free(res);
        break;
      case PROMPT_PASSWORD_CHAR:
        res = promptPassword(prompt_str+1);
        if(ipc_write(sock, res)!=0)
          return EXIT_FAILURE;
        memset(res, 0, strlen(res));
        free(res);
        break;
      default:
        syslog(LOG_AUTHPRIV|LOG_ALERT, "IPC Read malformed. Unknown mode %d %c.", *prompt_str, *prompt_str);
        free(prompt_str);
        return EXIT_FAILURE;
    }
    free(prompt_str);
  }
}
