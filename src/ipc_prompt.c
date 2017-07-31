#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <stdio.h>
#include <unistd.h>

#include "ipc_prompt.h"
#include "config.h"


/** @fn void runPassprompt()
 * @brief opens a new terminal running the oidc-prompt binary
 */
void runPassprompt() {
  int pid = fork();
  if(pid==-1) {
    perror("fork");
    exit(EXIT_FAILURE);
  }
  if (pid==0) {
    char* fmt = "x-terminal-emulator -e %s/oidc-prompt/bin/oidc-prompt";
    const char* cwd = conf_getcwd();
    char* cmd = calloc(sizeof(char),strlen(fmt)+strlen(cwd)-2+1);
    sprintf(cmd, fmt, cwd);
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "running callback cmd: %s\n",cmd);
    system(cmd);
    free(cmd);
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "exiting callback\n");
    exit(EXIT_SUCCESS);
  }
}


