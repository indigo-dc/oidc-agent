#define _XOPEN_SOURCE
#include "printer.h"

#include "defines/settings.h"
#include "utils/json.h"
#include "utils/stringUtils.h"

#include <sys/types.h>

void printEnvs(const char* daemon_socket, pid_t daemon_pid, unsigned char quiet,
               unsigned char json) {
  if (!json) {
    if (daemon_socket != NULL) {
      printStdout("%s=%s; export %s;\n", OIDC_SOCK_ENV_NAME, daemon_socket,
                  OIDC_SOCK_ENV_NAME);
    }
    if (daemon_pid != 0) {
      printStdout("%s=%d; export %s;\n", OIDC_PID_ENV_NAME, daemon_pid,
                  OIDC_PID_ENV_NAME);
      if (!quiet) {
        printStdout("echo Agent pid $%s\n", OIDC_PID_ENV_NAME);
      }
    }
  } else {
    char*  pid_str = oidc_sprintf("%d", daemon_pid);
    cJSON* jsonP =
        generateJSONObject("socket", cJSON_String, daemon_socket ?: "", "dpid",
                           cJSON_String, pid_str, NULL);
    secFree(pid_str);
    char* jsonPrint = jsonToString(jsonP);
    secFreeJson(jsonP);
    printStdout("%s", jsonPrint);
    secFree(jsonPrint);
  }
}
