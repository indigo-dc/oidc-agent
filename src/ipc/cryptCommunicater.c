#include "cryptCommunicater.h"
#include "communicator.h"
#include "utils/crypt.h"
#include "utils/memzero.h"
#include "utils/oidc_error.h"

#include <sodium.h>
#include <syslog.h>

char* ipc_cryptCommunicate(char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  return ipc_vcryptCommunicate(fmt, args);
}

char* ipc_vcryptCommunicate(char* fmt, va_list args) {
  va_list original;
  va_copy(original, args);

  unsigned char client_pk[crypto_kx_PUBLICKEYBYTES],
      client_sk[crypto_kx_SECRETKEYBYTES];
  unsigned char client_rx[crypto_kx_SESSIONKEYBYTES],
      client_tx[crypto_kx_SESSIONKEYBYTES];

  crypto_kx_keypair(client_pk, client_sk);
  // send public key to oidc-agent
  char* client_pk_base64 = toBase64((char*)client_pk, crypto_kx_PUBLICKEYBYTES);
  char* server_pk_base64 = ipc_communicate(client_pk_base64);
  secFree(client_pk_base64);
  if (server_pk_base64 == NULL) {
    moresecure_memzero(client_sk, crypto_kx_SECRETKEYBYTES);
    return NULL;
  }
  unsigned char server_pk[crypto_kx_PUBLICKEYBYTES];
  fromBase64(server_pk_base64, crypto_kx_PUBLICKEYBYTES, server_pk);
  secFree(server_pk_base64);
  if (crypto_kx_client_session_keys(client_rx, client_tx, client_pk, client_sk,
                                    server_pk) != 0) {
    /* Suspicious server public key, bail out */
    oidc_errno = OIDC_ECRYPPUB;
    moresecure_memzero(client_sk, crypto_kx_SECRETKEYBYTES);
    moresecure_memzero(client_rx, crypto_kx_SESSIONKEYBYTES);
    moresecure_memzero(client_tx, crypto_kx_SESSIONKEYBYTES);
    return NULL;
  }
  char* msg = secAlloc(sizeof(char) * (vsnprintf(NULL, 0, fmt, args) + 1));
  if (msg == NULL) {
    oidc_errno = OIDC_EALLOC;
    moresecure_memzero(client_sk, crypto_kx_SECRETKEYBYTES);
    moresecure_memzero(client_rx, crypto_kx_SESSIONKEYBYTES);
    moresecure_memzero(client_tx, crypto_kx_SESSIONKEYBYTES);
    return NULL;
  }
  vsprintf(msg, fmt, original);
  size_t                tx_msg_len = strlen(msg);
  struct encryptionInfo cryptResult =
      crypt_encryptWithKey((unsigned char*)msg, client_tx);
  secFree(msg);
  if (cryptResult.encrypted_base64 == NULL) {
    secFreeEncryptionInfo(cryptResult);
    moresecure_memzero(client_sk, crypto_kx_SECRETKEYBYTES);
    moresecure_memzero(client_rx, crypto_kx_SESSIONKEYBYTES);
    moresecure_memzero(client_tx, crypto_kx_SESSIONKEYBYTES);
    return NULL;
  }
  char* encryptedMessage =
      oidc_sprintf("%lu:%s:%s", tx_msg_len, cryptResult.nonce_base64,
                   cryptResult.encrypted_base64);
  secFreeEncryptionInfo(cryptResult);
  if (encryptedMessage == NULL) {
    moresecure_memzero(client_sk, crypto_kx_SECRETKEYBYTES);
    moresecure_memzero(client_rx, crypto_kx_SESSIONKEYBYTES);
    moresecure_memzero(client_tx, crypto_kx_SESSIONKEYBYTES);
    return NULL;
  }
  char* encryptedResponse = ipc_communicate(encryptedMessage);
  secFree(encryptedMessage);
  if (encryptedResponse == NULL) {
    moresecure_memzero(client_sk, crypto_kx_SECRETKEYBYTES);
    moresecure_memzero(client_rx, crypto_kx_SESSIONKEYBYTES);
    moresecure_memzero(client_tx, crypto_kx_SESSIONKEYBYTES);
    return NULL;
  }
  size_t rx_msg_len;
  char*  res_nonce_base64     = strtok(encryptedResponse, ":");
  char*  res_encrypted_base64 = strtok(NULL, ":");
  sscanf(encryptedResponse, "%lu", &rx_msg_len);
  if (res_nonce_base64 == NULL || res_encrypted_base64 == NULL) {
    moresecure_memzero(client_sk, crypto_kx_SECRETKEYBYTES);
    moresecure_memzero(client_rx, crypto_kx_SESSIONKEYBYTES);
    moresecure_memzero(client_tx, crypto_kx_SESSIONKEYBYTES);
    oidc_errno = OIDC_ECRYPMIPC;
    return NULL;
  }
  struct encryptionInfo crypt = {.nonce_base64     = res_nonce_base64,
                                 .encrypted_base64 = res_encrypted_base64};
  unsigned char*        decryptedResponse =
      crypt_decryptWithKey(crypt, rx_msg_len, client_rx);
  secFree(encryptedResponse);

  moresecure_memzero(client_sk, crypto_kx_SECRETKEYBYTES);
  moresecure_memzero(client_rx, crypto_kx_SESSIONKEYBYTES);
  moresecure_memzero(client_tx, crypto_kx_SESSIONKEYBYTES);
  return (char*)decryptedResponse;
}
