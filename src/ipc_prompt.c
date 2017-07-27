#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <stdio.h>
#include <unistd.h>

#include "ipc_prompt.h"
#include "config.h"
#include "ipc.h"

/** @fn char* getUserInput(char* prompt, int mode)
 * @brief prompts the user for input and returns it using ipc
 * @param prompt the prompt string
 * @param mode specifies if the user input should be visible or not. If 0 the
 * user input is visible. Otherwise terminal echo will be disabled
 * @return a pointer to the user input. Has to be freed after usage
 */
char* getUserInput(char* prompt, int mode) {
  ipc_close();
  ipc_init();
  int msgsock = ipc_bind(runPassprompt);
  char* toSend = calloc(sizeof(char), strlen(prompt)+1+1);
  sprintf(toSend, "%d%s" ,mode?1:0, prompt);
  if(ipc_write(msgsock, toSend)!=0) { 
    free(toSend);
    return NULL;
  }
  free(toSend);
  char* res = ipc_read(msgsock);
  ipc_close();
  return res;
}

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

