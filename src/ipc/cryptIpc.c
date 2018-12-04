#include "cryptIpc.h"
#include "ipc.h"
#include "utils/crypt.h"
#include "utils/json.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"

#include <sodium.h>
#include <stdio.h>
#include <syslog.h>

oidc_error_t server_ipc_cryptWrite(int sock, const unsigned char* key,
                                   char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  return server_ipc_vcryptWrite(sock, key, fmt, args);
}

oidc_error_t server_ipc_vcryptWrite(int sock, const unsigned char* key,
                                    char* fmt, va_list args) {
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
  ipc_write(sock, encryptedMessage);
  secFree(encryptedMessage);
  return OIDC_SUCCESS;
}

static char          encrypt = 0;
static unsigned char server_rx[crypto_kx_SESSIONKEYBYTES],
    server_tx[crypto_kx_SESSIONKEYBYTES];

oidc_error_t server_ipc_write(int sock, char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  if (!encrypt) {
    return ipc_vwrite(sock, fmt, args);
  }
  encrypt        = 0;
  oidc_error_t e = server_ipc_vcryptWrite(sock, server_tx, fmt, args);
  moresecure_memzero(server_tx, crypto_kx_SESSIONKEYBYTES);
  if (e == OIDC_SUCCESS) {
    return OIDC_SUCCESS;
  }
  return ipc_writeOidcErrno(sock);
}

char* server_ipc_cryptRead(int sock, const char* msg) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Doing encrypted ipc read");
  unsigned char client_pk[crypto_kx_PUBLICKEYBYTES];
  fromBase64(msg, crypto_kx_PUBLICKEYBYTES, client_pk);
  unsigned char server_pk[crypto_kx_PUBLICKEYBYTES],
      server_sk[crypto_kx_SECRETKEYBYTES];

  crypto_kx_keypair(server_pk, server_sk);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Generated server sec/pub keys");

  if (crypto_kx_server_session_keys(server_rx, server_tx, server_pk, server_sk,
                                    client_pk) != 0) {
    /* Suspicious client public key, bail out */
    oidc_errno = OIDC_ECRYPPUB;
    moresecure_memzero(server_sk, crypto_kx_SECRETKEYBYTES);
    moresecure_memzero(server_rx, crypto_kx_SESSIONKEYBYTES);
    moresecure_memzero(server_tx, crypto_kx_SESSIONKEYBYTES);
    return NULL;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG,
         "Successfully generated server session keys");
  char* server_pk_base64 = toBase64((char*)server_pk, crypto_kx_PUBLICKEYBYTES);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Communicating server pub key");
  char* encrypted_request = ipc_communicateWithSock(sock, server_pk_base64);
  secFree(server_pk_base64);
  if (encrypted_request == NULL) {
    moresecure_memzero(server_sk, crypto_kx_SECRETKEYBYTES);
    moresecure_memzero(server_rx, crypto_kx_SESSIONKEYBYTES);
    moresecure_memzero(server_tx, crypto_kx_SESSIONKEYBYTES);
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
    moresecure_memzero(server_rx, crypto_kx_SESSIONKEYBYTES);
    moresecure_memzero(server_tx, crypto_kx_SESSIONKEYBYTES);
    oidc_errno = OIDC_ECRYPMIPC;
    return NULL;
  }
  struct encryptionInfo crypt             = {.nonce_base64     = req_nonce_base64,
                                 .cryptParameter   = newCryptParameters(),
                                 .encrypted_base64 = req_encrypted_base64};
  unsigned char*        decryptedResponse = crypt_decryptWithKey(
      crypt, rx_msg_len + crypt.cryptParameter.mac_len, server_rx);
  secFree(encrypted_request);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Decrypted request is '%s'",
         decryptedResponse);
  moresecure_memzero(server_sk, crypto_kx_SECRETKEYBYTES);
  moresecure_memzero(server_rx, crypto_kx_SESSIONKEYBYTES);
  if (decryptedResponse != NULL) {
    encrypt = 1;
  }
  return (char*)decryptedResponse;
}

char* server_ipc_read(int sock) {
  char* msg = ipc_read(sock);
  if (isJSONObject(msg)) {
    return msg;
  }
  char* res = server_ipc_cryptRead(sock, msg);
  secFree(msg);
  if (res == NULL) {
    ipc_writeOidcErrno(sock);
  }
  return res;
}

oidc_error_t server_ipc_writeOidcErrno(int sock) {
  return server_ipc_write(sock, RESPONSE_ERROR, oidc_serror());
}
