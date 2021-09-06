#include "cryptIpc.h"
#include "ipc.h"
#include "utils/crypt/crypt.h"
#include "utils/crypt/ipcCryptUtils.h"
#include "utils/json.h"
#include "utils/logger.h"
#include "utils/memory.h"
#include "utils/memzero.h"
#include "utils/oidc_error.h"
#include "utils/stringUtils.h"

#include <sodium.h>
#include <stdio.h>
#include <string.h>

#include <winsock2.h>


typedef int (*crypto_kx_session_keys)(
    unsigned char       rx[crypto_kx_SESSIONKEYBYTES],
    unsigned char       tx[crypto_kx_SESSIONKEYBYTES],
    const unsigned char pk[crypto_kx_PUBLICKEYBYTES],
    const unsigned char sk[crypto_kx_SECRETKEYBYTES],
    const unsigned char other_pk[crypto_kx_PUBLICKEYBYTES]);


oidc_error_t ipc_cryptWrite(const SOCKET sock, const unsigned char* key,
                            const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  oidc_error_t ret = ipc_vcryptWrite(sock, key, fmt, args);
  va_end(args);
  return ret;
}

oidc_error_t ipc_vcryptWrite(const SOCKET sock, const unsigned char* key,
                             const char* fmt, va_list args) {
  char* msg = oidc_vsprintf(fmt, args);
  if (msg == NULL) {
    return oidc_errno;
  }
  logger(DEBUG, "Doing encrypted ipc write of %lu bytes: '%s'", strlen(msg),
         msg);
  char* encryptedMessage = encryptForIpc(msg, key);
  secFree(msg);
  if (encryptedMessage == NULL) {
    return oidc_errno;
  }
  oidc_error_t e = ipc_write(sock, encryptedMessage);
  secFree(encryptedMessage);
  return e;
}

void secFreePubSecKeySet(struct pubsec_keySet* k) { secFree(k); }

struct pubsec_keySet* generatePubSecKeys() {
  struct pubsec_keySet* keys = secAlloc(sizeof(struct pubsec_keySet));
  crypto_kx_keypair(keys->pk, keys->sk);
  logger(DEBUG, "Generated pub/sec keys");
  return keys;
}

char* communicatePublicKey(const SOCKET _sock, const char* publicKey) {
  if (publicKey == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  char* pk_base64 = toBase64(publicKey, crypto_kx_PUBLICKEYBYTES);
  logger(DEBUG, "Communicating pub key");

  char* res = ipc_communicateWithSock(_sock, pk_base64);
  secFree(pk_base64);
  return res;
}

unsigned char* generateIpcKey(const unsigned char* publicKey,
                              const unsigned char* privateKey) {
  if (publicKey == NULL || privateKey == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  unsigned char* sharedKey = secAlloc(crypto_box_BEFORENMBYTES);
  if (crypto_box_beforenm(sharedKey, publicKey, privateKey) != 0) {
    oidc_errno = OIDC_ECRYPPUB;
    secFree(sharedKey);
    return NULL;
  }
  return sharedKey;
}

list_t* encryptionKeys = NULL;

char* server_ipc_cryptRead(const SOCKET sock, const char* client_pk_base64) {
  logger(DEBUG, "Doing encrypted ipc read");
  unsigned char client_pk[crypto_kx_PUBLICKEYBYTES];
  fromBase64(client_pk_base64, crypto_kx_PUBLICKEYBYTES, client_pk);
  struct pubsec_keySet* pubsec_keys = generatePubSecKeys();
  unsigned char*        ipc_key = generateIpcKey(client_pk, pubsec_keys->sk);
  if (ipc_key == NULL) {
    secFree(ipc_key);
    return NULL;
  }
  char* encrypted_request = communicatePublicKey(sock, (char*)pubsec_keys->pk);
  secFreePubSecKeySet(pubsec_keys);
  if (encrypted_request == NULL) {
    secFree(ipc_key);
    return NULL;
  }
  logger(DEBUG, "Received encrypted request");
  char* decryptedRequest = decryptForIpc(encrypted_request, ipc_key);
  secFree(encrypted_request);
  logger(DEBUG, "Decrypted request is '%s'", decryptedRequest);
  if (decryptedRequest != NULL) {
    if (encryptionKeys == NULL) {
      encryptionKeys = list_new();
    }
    list_rpush(encryptionKeys, list_node_new(ipc_key));
  } else {
    secFree(ipc_key);
  }
  return decryptedRequest;
}

unsigned char* client_keyExchange(const SOCKET sock) {
  struct pubsec_keySet* pubsec_keys = generatePubSecKeys();
  char* server_pk_base64 = communicatePublicKey(sock, (char*)pubsec_keys->pk);
  if (server_pk_base64 == NULL) {
    secFreePubSecKeySet(pubsec_keys);
    return NULL;
  }
  logger(DEBUG, "Received server public key");
  unsigned char server_pk[crypto_kx_PUBLICKEYBYTES];
  fromBase64(server_pk_base64, crypto_kx_PUBLICKEYBYTES, server_pk);
  secFree(server_pk_base64);
  unsigned char* ipc_key = generateIpcKey(server_pk, pubsec_keys->sk);
  secFreePubSecKeySet(pubsec_keys);
  if (ipc_key == NULL) {
    return NULL;
  }
  return ipc_key;
}
