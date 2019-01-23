#include "cryptIpc.h"
#include "ipc.h"

#include "list/list.h"

list_t* encryptionKeys = NULL;

oidc_error_t server_ipc_write(const int sock, char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  if (encryptionKeys == NULL || encryptionKeys->len <= 0) {
    return ipc_vwrite(sock, fmt, args);
  }
  list_node_t*       node = list_rpop(encryptionKeys);
  struct ipc_keySet* keys = node->val;
  LIST_FREE(node);

  oidc_error_t e = ipc_vcryptWrite(sock, keys->key_tx, fmt, args);
  secFree(keys);
  if (e == OIDC_SUCCESS) {
    return OIDC_SUCCESS;
  }
  return ipc_writeOidcErrno(sock);
}
