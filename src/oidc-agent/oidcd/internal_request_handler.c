#include "internal_request_handler.h"

#include "defines/ipc_values.h"
#include "ipc/pipe.h"
#include "utils/agentLogger.h"
#include "utils/memory.h"
#include "utils/parseJson.h"

void oidcd_handleUpdateRefreshToken(const struct ipcPipe pipes,
                                    const char*          short_name,
                                    const char*          refresh_token) {
  char* res   = ipc_communicateThroughPipe(pipes, INT_REQUEST_UPD_REFRESH,
                                           short_name, refresh_token);
  char* error = parseForError(res);
  if (error == NULL) {
    agent_log(DEBUG, "Successfully updated refresh token for '%s'", short_name);
    return;
  }
  secFree(error);
  agent_log(
      WARNING,
      "WARNING: Received new refresh token from OIDC Provider. It's most "
      "likely that the old one was therefore revoked. Updating the config "
      "file failed. You may want to revoke the new refresh token or pass it "
      "to oidc-gen --rt");
}
