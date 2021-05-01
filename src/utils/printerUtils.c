#define _XOPEN_SOURCE
#include "printer.h"
#include "json.h"
#include "defines/settings.h"

#include <sys/types.h>

void printEnvs(char* daemon_socket, pid_t daemon_pid, unsigned char quiet, unsigned char json) {
  if(!json) {
    if (daemon_socket != NULL) {
      printStdout("%s=%s; export %s;\n", OIDC_SOCK_ENV_NAME, daemon_socket, OIDC_SOCK_ENV_NAME);
    }
    if(daemon_pid != 0) {
      printStdout("%s=%d; export %s;\n", OIDC_PID_ENV_NAME, daemon_pid, OIDC_PID_ENV_NAME);
      if (!quiet) {
        printStdout("echo Agent pid $%s\n", OIDC_PID_ENV_NAME);
      }
    }
  } else {
    // Convert pid_t to string...
    char *pid_str = malloc(snprintf(NULL, 0, "%d", daemon_pid) + 1);
    sprintf(pid_str, "%d", daemon_pid);

    // print empty string of daemon_socket is null
    char *daemon_socket_str = daemon_socket;
    if (daemon_socket_str == NULL) {
      daemon_socket_str = "";
    }
    
    cJSON *outer_json = generateJSONObject("socket", cJSON_String, daemon_socket_str, "dpid", cJSON_String, pid_str, NULL);
    printStdout("%s", jsonToString(outer_json));
  }
}
