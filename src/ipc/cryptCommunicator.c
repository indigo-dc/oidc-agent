#include "cryptCommunicator.h"
#include "ipc.h"
#include "settings.h"
#include "utils/crypt/crypt.h"
#include "utils/json.h"
#include "utils/memzero.h"
#include "utils/oidc_error.h"

#include <sodium.h>
#include <syslog.h>

char* ipc_cryptCommunicate(char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  return ipc_vcryptCommunicate(fmt, args);
}

void secFreeIpcKeySet(struct ipc_keySet* k) { secFree(k); }

struct ipc_keySet* client_keyExchange(int rx, int tx) {
  unsigned char client_pk[crypto_kx_PUBLICKEYBYTES],
      client_sk[crypto_kx_SECRETKEYBYTES];
  crypto_kx_keypair(client_pk, client_sk);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Generated client pub/sec keys");
  // send public key to oidc-agent
  char* client_pk_base64 = toBase64((char*)client_pk, crypto_kx_PUBLICKEYBYTES);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Sending client public key");
  char* server_pk_base64 =
      ipc_communicateWithSockPair(rx, tx, client_pk_base64);
  secFree(client_pk_base64);
  if (server_pk_base64 == NULL) {
    moresecure_memzero(client_sk, crypto_kx_SECRETKEYBYTES);
    return NULL;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Received server public key");
  unsigned char server_pk[crypto_kx_PUBLICKEYBYTES];
  fromBase64(server_pk_base64, crypto_kx_PUBLICKEYBYTES, server_pk);
  secFree(server_pk_base64);
  struct ipc_keySet* ipc_keys = secAlloc(sizeof(struct ipc_keySet));
  if (crypto_kx_client_session_keys(ipc_keys->key_rx, ipc_keys->key_tx,
                                    client_pk, client_sk, server_pk) != 0) {
    /* Suspicious server public key, bail out */
    oidc_errno = OIDC_ECRYPPUB;
    moresecure_memzero(client_sk, crypto_kx_SECRETKEYBYTES);
    secFreeIpcKeySet(ipc_keys);
    return NULL;
  }
  moresecure_memzero(client_sk, crypto_kx_SECRETKEYBYTES);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Generated client session keys");
  return ipc_keys;
}

struct ipc_keySet* client_ipc_writeToSock(int rx, int tx, char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  return client_ipc_vwriteToSock(rx, tx, fmt, args);
}

struct ipc_keySet* client_ipc_vwriteToSock(int rx, int tx, char* fmt,
                                           va_list args) {
  va_list original;
  va_copy(original, args);
  char* msg = secAlloc(sizeof(char) * (vsnprintf(NULL, 0, fmt, args) + 1));
  if (msg == NULL) {
    oidc_errno = OIDC_EALLOC;
    return NULL;
  }
  vsprintf(msg, fmt, original);
  size_t tx_msg_len = strlen(msg);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Sending %lu bytes encrypted: '%s'",
         tx_msg_len, msg);
  struct ipc_keySet* ipc_keys = client_keyExchange(rx, tx);
  if (ipc_keys == NULL) {
    secFree(msg);
    return NULL;
  }
  struct encryptionInfo cryptResult =
      crypt_encryptWithKey((unsigned char*)msg, ipc_keys->key_tx);
  secFree(msg);
  if (cryptResult.encrypted_base64 == NULL) {
    secFreeEncryptionInfo(cryptResult);
    secFreeIpcKeySet(ipc_keys);
    return NULL;
  }
  oidc_error_t e =
      ipc_write(tx, "%lu:%s:%s", tx_msg_len, cryptResult.nonce_base64,
                cryptResult.encrypted_base64);
  secFreeEncryptionInfo(cryptResult);
  if (e != OIDC_SUCCESS) {
    secFreeIpcKeySet(ipc_keys);
  }
  return ipc_keys;
}

char* ipc_vcryptCommunicate(char* fmt, va_list args) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Doing encrypted ipc communication");
  static struct connection con;
  if (ipc_init(&con, OIDC_SOCK_ENV_NAME, 0) != OIDC_SUCCESS) {
    return NULL;
  }
  if (ipc_connect(con) < 0) {
    return NULL;
  }

  struct ipc_keySet* ipc_keys =
      client_ipc_vwriteToSock(*(con.sock), *(con.sock), fmt, args);
  if (ipc_keys == NULL) {
    ipc_close(&con);
    return NULL;
  }
  char* encryptedResponse = ipc_read(*(con.sock));
  ipc_close(&con);
  if (encryptedResponse == NULL) {
    secFreeIpcKeySet(ipc_keys);
    return NULL;
  }

  if (isJSONObject(encryptedResponse)) {
    // Response not encrypted
    secFreeIpcKeySet(ipc_keys);
    return encryptedResponse;
  }

  size_t rx_msg_len;
  char*  len_str              = strtok(encryptedResponse, ":");
  char*  res_nonce_base64     = strtok(NULL, ":");
  char*  res_encrypted_base64 = strtok(NULL, ":");
  sscanf(len_str, "%lu", &rx_msg_len);
  if (res_nonce_base64 == NULL || res_encrypted_base64 == NULL) {
    secFreeIpcKeySet(ipc_keys);
    oidc_errno = OIDC_ECRYPMIPC;
    return NULL;
  }
  struct encryptionInfo crypt             = {.nonce_base64     = res_nonce_base64,
                                 .encrypted_base64 = res_encrypted_base64,
                                 .cryptParameter   = newCryptParameters()};
  unsigned char*        decryptedResponse = crypt_decryptWithKey(
      crypt, rx_msg_len + crypt.cryptParameter.mac_len, ipc_keys->key_rx);
  secFree(encryptedResponse);

  secFreeIpcKeySet(ipc_keys);
  return (char*)decryptedResponse;
}
