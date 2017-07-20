#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>

#include "../../src/ipc.h"
#include "prompt.h"

int main(int argc, char *argv[]) {
  openlog("oidc-prompt", LOG_CONS|LOG_PID, LOG_AUTHPRIV);
  // setlogmask(LOG_UPTO(LOG_DEBUG));
  setlogmask(LOG_UPTO(LOG_NOTICE));
  ipc_init();
  int sock = ipc_connect();
  char* prompt = ipc_read(sock);
  if(prompt==NULL) {
    return EXIT_FAILURE;
  }
  char* password = promptPassword(prompt);
  free(prompt);
  if(ipc_write(sock, password)!=0)
    return EXIT_FAILURE;
  free(password);
  ipc_close();
  return EXIT_SUCCESS;
}
