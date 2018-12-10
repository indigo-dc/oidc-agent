#include "cryptIpc.h"
#include "ipc.h"
#include "utils/crypt.h"
#include "utils/json.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"

#include <sodium.h>
#include <stdio.h>
#include <syslog.h>

oidc_error_t server_ipc_cryptWrite(int tx, const unsigned char* key, char* fmt,
                                   ...) {
  va_list args;
  va_start(args, fmt);
  return server_ipc_vcryptWrite(tx, key, fmt, args);
}

oidc_error_t server_ipc_vcryptWrite(int tx, const unsigned char* key, char* fmt,
                                    va_list args) {
  va_list original;
  va_copy(original, args);
  char* msg = secAlloc(sizeof(char) * (vsnprintf(NULL, 0, fmt, args) + 1));
  if (msg == NULL) {
    oidc_errno = OIDC_EALLOC;
    return oidc_errno;
  }
  vsprintf(msg, fmt, original);
  size_t tx_msg_len = strlen(msg);
  syslog(LOG_AUTHPRIV | LOG_DEBUG,
         "Doing encrypted ipc write of %lu bytes: '%s'", tx_msg_len, msg);
  struct encryptionInfo cryptResult =
      crypt_encryptWithKey((unsigned char*)msg, key);
  secFree(msg);
  if (cryptResult.encrypted_base64 == NULL) {
    secFreeEncryptionInfo(cryptResult);
    return oidc_errno;
  }
  char* encryptedMessage =
      oidc_sprintf("%lu:%s:%s", tx_msg_len, cryptResult.nonce_base64,
                   cryptResult.encrypted_base64);
  secFreeEncryptionInfo(cryptResult);
  if (encryptedMessage == NULL) {
    return oidc_errno;
  }
  ipc_write(tx, encryptedMessage);
  secFree(encryptedMessage);
  return OIDC_SUCCESS;
}

struct ipc_keySet {
  unsigned char key_rx[crypto_kx_SESSIONKEYBYTES];
  unsigned char key_tx[crypto_kx_SESSIONKEYBYTES];
};

list_t* encryptionKeys = NULL;

oidc_error_t server_ipc_write(int tx, char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  if (encryptionKeys == NULL || encryptionKeys->len <= 0) {
    return ipc_vwrite(tx, fmt, args);
  }
  list_node_t*       node = list_rpop(encryptionKeys);
  struct ipc_keySet* keys = node->val;
  LIST_FREE(node);

  oidc_error_t e = server_ipc_vcryptWrite(tx, keys->key_tx, fmt, args);
  secFree(keys);
  if (e == OIDC_SUCCESS) {
    return OIDC_SUCCESS;
  }
  return ipc_writeOidcErrno(tx);
}

char* server_ipc_cryptRead(int rx, int tx, const char* msg) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Doing encrypted ipc read");
  unsigned char client_pk[crypto_kx_PUBLICKEYBYTES];
  fromBase64(msg, crypto_kx_PUBLICKEYBYTES, client_pk);
  unsigned char server_pk[crypto_kx_PUBLICKEYBYTES],
      server_sk[crypto_kx_SECRETKEYBYTES];

  crypto_kx_keypair(server_pk, server_sk);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Generated server sec/pub keys");
  struct ipc_keySet* keys = secAlloc(sizeof(struct ipc_keySet));

  if (crypto_kx_server_session_keys(keys->key_rx, keys->key_tx, server_pk,
                                    server_sk, client_pk) != 0) {
    /* Suspicious client public key, bail out */
    oidc_errno = OIDC_ECRYPPUB;
    moresecure_memzero(server_sk, crypto_kx_SECRETKEYBYTES);
    secFree(keys);
    return NULL;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG,
         "Successfully generated server session keys");
  char* server_pk_base64 = toBase64((char*)server_pk, crypto_kx_PUBLICKEYBYTES);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Communicating server pub key");
  char* encrypted_request =
      ipc_communicateWithSockPair(rx, tx, server_pk_base64);
  secFree(server_pk_base64);
  if (encrypted_request == NULL) {
    moresecure_memzero(server_sk, crypto_kx_SECRETKEYBYTES);
    secFree(keys);
    return NULL;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Received encrypted request");

  size_t rx_msg_len;
  char*  len_str              = strtok(encrypted_request, ":");
  char*  req_nonce_base64     = strtok(NULL, ":");
  char*  req_encrypted_base64 = strtok(NULL, ":");
  sscanf(len_str, "%lu", &rx_msg_len);
  if (req_nonce_base64 == NULL || req_encrypted_base64 == NULL) {
    moresecure_memzero(server_sk, crypto_kx_SECRETKEYBYTES);
    secFree(keys);
    oidc_errno = OIDC_ECRYPMIPC;
    return NULL;
  }
  struct encryptionInfo crypt             = {.nonce_base64     = req_nonce_base64,
                                 .cryptParameter   = newCryptParameters(),
                                 .encrypted_base64 = req_encrypted_base64};
  unsigned char*        decryptedResponse = crypt_decryptWithKey(
      crypt, rx_msg_len + crypt.cryptParameter.mac_len, keys->key_rx);
  secFree(encrypted_request);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Decrypted request is '%s'",
         decryptedResponse);
  moresecure_memzero(server_sk, crypto_kx_SECRETKEYBYTES);
  if (decryptedResponse != NULL) {
    if (encryptionKeys == NULL) {
      encryptionKeys = list_new();
    }
    list_rpush(encryptionKeys, list_node_new(keys));
  } else {
    secFree(keys);
  }
  return (char*)decryptedResponse;
}

char* server_ipc_read(int rx, int tx) {
  char* msg = ipc_read(rx);
  if (isJSONObject(msg)) {
    return msg;
  }
  char* res = server_ipc_cryptRead(rx, tx, msg);
  secFree(msg);
  if (res == NULL) {
    ipc_writeOidcErrno(tx);
  }
  return res;
}

void server_ipc_freeLastKey() {
  if (encryptionKeys == NULL || encryptionKeys->len <= 0) {
    return;
  }
  list_node_t*       node = list_rpop(encryptionKeys);
  struct ipc_keySet* keys = node->val;
  LIST_FREE(node);
  secFree(keys);
}

char* server_ipc_readFromSocket(int sock) {
  return server_ipc_read(sock, sock);
}

oidc_error_t server_ipc_writeOidcErrno(int tx) {
  return server_ipc_write(tx, RESPONSE_ERROR, oidc_serror());
}
