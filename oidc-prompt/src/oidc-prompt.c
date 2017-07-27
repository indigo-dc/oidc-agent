#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "../../src/ipc.h"
#include "prompt.h"

int main(/* int argc,char** argv */) {
  openlog("oidc-prompt", LOG_CONS|LOG_PID, LOG_AUTHPRIV);
  // setlogmask(LOG_UPTO(LOG_DEBUG));
  setlogmask(LOG_UPTO(LOG_NOTICE));
  ipc_init();
  int sock = ipc_connect();
  char* prompt_str = ipc_read(sock);
  if(prompt_str==NULL) {
    return EXIT_FAILURE;
  }
  char* res;
  switch(*prompt_str) {
    case '0':
      res = prompt(prompt_str+1);
      break;
    case '1':
      res = promptPassword(prompt_str+1);
      break;
    default:
      syslog(LOG_AUTHPRIV|LOG_ALERT, "IPC Read malformed. Unknown mode.");
      free(prompt_str);
      return EXIT_SUCCESS;
  }
  free(prompt_str);
  if(ipc_write(sock, res)!=0)
    return EXIT_FAILURE;
  memset(res, 0, strlen(res));
  free(res);
  ipc_close();
  return EXIT_SUCCESS;
}
