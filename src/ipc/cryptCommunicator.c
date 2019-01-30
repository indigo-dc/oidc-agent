#include "cryptCommunicator.h"
#include "cryptIpc.h"
#include "ipc.h"
#include "settings.h"
#include "utils/crypt/cryptUtils.h"
#include "utils/json.h"
#include "utils/oidc_error.h"

#include <sodium.h>
#include <syslog.h>

char* ipc_cryptCommunicate(char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char* ret = ipc_vcryptCommunicate(fmt, args);
  va_end(args);
  return ret;
}

// struct ipc_keySet* client_ipc_write(const int sock, const char* fmt, ...) {
//   va_list args;
//   va_start(args, fmt);
//   struct ipc_keySet* ret = client_ipc_vwrite(sock, fmt, args);
//   va_end(args);
//   return ret;
// }
//
// struct ipc_keySet* client_ipc_vwrite(const int sock, const char* fmt,
//                                      va_list args) {
//   struct ipc_keySet* ipc_keys = client_keyExchange(sock);
//   if (ipc_keys == NULL) {
//     return NULL;
//   }
//   oidc_error_t e = ipc_vcryptWrite(sock, ipc_keys->key_tx, fmt, args);
//   if (e != OIDC_SUCCESS) {
//     secFreeIpcKeySet(ipc_keys);
//     return NULL;
//   }
//   return ipc_keys;
// }

char* ipc_vcryptCommunicate(char* fmt, va_list args) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Doing encrypted ipc communication");
  static struct connection con;
  if (ipc_client_init(&con, OIDC_SOCK_ENV_NAME) != OIDC_SUCCESS) {
    return NULL;
  }
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
