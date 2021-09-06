#include "cryptCommunicator.h"
#include "cryptIpc.h"
#include "defines/settings.h"
#include "ipc.h"
#include "utils/crypt/ipcCryptUtils.h"
#include "utils/json.h"
#include "utils/logger.h"
#include "utils/oidc_error.h"

#include <sodium.h>

char* _ipc_vcryptCommunicateWithConnection(struct connection con,
                                           const char* fmt, va_list args) {
  logger(DEBUG, "Doing encrypted ipc communication");
  if (ipc_connect(con) < 0) {
    return NULL;
  }
  oidc_error_t ret = ipc_msys_authorize(con);
  if (ret != OIDC_SUCCESS) {
    return NULL;
  }
  
  unsigned char* ipc_key = client_keyExchange(*(con.sock));
  if (ipc_key == NULL) {
    ipc_closeConnection(&con);
    return NULL;
  }
  oidc_error_t e = ipc_vcryptWrite(*(con.sock), ipc_key, fmt, args);
  if (e != OIDC_SUCCESS) {
    secFree(ipc_key);
    ipc_closeConnection(&con);
    return NULL;
  }

  char* encryptedResponse = ipc_read(*(con.sock));
  ipc_closeConnection(&con);
  if (encryptedResponse == NULL) {
    secFree(ipc_key);
    return NULL;
  }

  if (isJSONObject(encryptedResponse)) {
    // Response not encrypted
    secFree(ipc_key);
    return encryptedResponse;
  }
  char* decryptedResponse = decryptForIpc(encryptedResponse, ipc_key);
  secFree(encryptedResponse);
  secFree(ipc_key);
  return decryptedResponse;
}

char* ipc_vcryptCommunicate(unsigned char remote, const char* fmt,
                            va_list args) {
  static struct connection con;
  if (ipc_client_init(&con, remote) != OIDC_SUCCESS) {
    return NULL;
  }
  return _ipc_vcryptCommunicateWithConnection(con, fmt, args);
}
