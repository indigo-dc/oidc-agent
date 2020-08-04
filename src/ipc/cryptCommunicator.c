#include "cryptCommunicator.h"
#include "cryptIpc.h"
#include "defines/settings.h"
#include "ipc.h"
#include "utils/crypt/ipcCryptUtils.h"
#include "utils/json.h"
#include "utils/logger.h"
#include "utils/oidc_error.h"

#include <sodium.h>

char* ipc_cryptCommunicate(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char* ret = ipc_vcryptCommunicate(fmt, args);
  va_end(args);
  return ret;
}

char* _ipc_vcryptCommunicateWithConnection(struct connection con,
                                           const char* fmt, va_list args) {
  logger(DEBUG, "Doing encrypted ipc communication");
  if (ipc_connect(con) < 0) {
    return NULL;
  }
  struct ipc_keySet* ipc_keys = client_keyExchange(*(con.sock));
  if (ipc_keys == NULL) {
    ipc_closeConnection(&con);
    return NULL;
  }
  oidc_error_t e = ipc_vcryptWrite(*(con.sock), ipc_keys->key_tx, fmt, args);
  if (e != OIDC_SUCCESS) {
    secFreeIpcKeySet(ipc_keys);
    ipc_closeConnection(&con);
    return NULL;
  }

  char* encryptedResponse = ipc_read(*(con.sock));
  ipc_closeConnection(&con);
  if (encryptedResponse == NULL) {
    secFreeIpcKeySet(ipc_keys);
    return NULL;
  }

  if (isJSONObject(encryptedResponse)) {
    // Response not encrypted
    secFreeIpcKeySet(ipc_keys);
    return encryptedResponse;
  }
  char* decryptedResponse = decryptForIpc(encryptedResponse, ipc_keys->key_rx);
  secFree(encryptedResponse);
  secFreeIpcKeySet(ipc_keys);
  return decryptedResponse;
}

char* ipc_vcryptCommunicate(const char* fmt, va_list args) {
  static struct connection con;
  if (ipc_client_init(&con, OIDC_SOCK_ENV_NAME) != OIDC_SUCCESS) {
    return NULL;
  }
  return _ipc_vcryptCommunicateWithConnection(con, fmt, args);
}

char* ipc_vcryptCommunicateWithPath(const char* socket_path, const char* fmt,
                                    va_list args) {
  static struct connection con;
  if (initConnectionWithPath(&con, socket_path) != OIDC_SUCCESS) {
    return NULL;
  }
  return _ipc_vcryptCommunicateWithConnection(con, fmt, args);
}

char* ipc_cryptCommunicateWithPath(const char* socket_path, const char* fmt,
                                   ...) {
  va_list args;
  va_start(args, fmt);
  return ipc_vcryptCommunicateWithPath(socket_path, fmt, args);
}
