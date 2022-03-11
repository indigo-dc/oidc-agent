#include "cryptCommunicator.h"

#include "cryptIpc.h"
#include "ipc.h"
#include "utils/crypt/ipcCryptUtils.h"
#include "utils/json.h"
#include "utils/logger.h"
#include "utils/oidc_error.h"

char* _ipc_vcryptCommunicateWithConnection(struct connection con,
                                           const char* fmt, va_list args) {
  logger(DEBUG, "Doing encrypted ipc communication");
  if (ipc_connect(con) != OIDC_SUCCESS) {
    return NULL;
  }
#ifdef __MINGW32__
  if (ipc_msys_authorize(con) != OIDC_SUCCESS) {
      return NULL;
  }
#endif
  unsigned char* ipc_key = client_keyExchange(*(con.sock));
  if (ipc_key == NULL) {
    ipc_closeConnection(&con);
    return NULL;
  }
  if (ipc_vcryptWrite(*(con.sock), ipc_key, fmt, args) != OIDC_SUCCESS) {
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

char* ipc_cryptCommunicate(unsigned char remote, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char* ret = ipc_vcryptCommunicate(remote, fmt, args);
  va_end(args);
  return ret;
}

char* ipc_vcryptCommunicate(unsigned char remote, const char* fmt,
                            va_list args) {
  static struct connection con;
  if (ipc_client_init(&con, remote) != OIDC_SUCCESS) {
    return NULL;
  }
  return _ipc_vcryptCommunicateWithConnection(con, fmt, args);
}

#ifndef __MINGW32__
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
#endif