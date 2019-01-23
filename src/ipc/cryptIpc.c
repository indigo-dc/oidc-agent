#include "cryptIpc.h"
#include "ipc.h"
#include "utils/crypt/cryptUtils.h"
#include "utils/json.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"

#include <sodium.h>
#include <stdio.h>
#include <syslog.h>

typedef int (*crypto_kx_session_keys)(
    unsigned char       rx[crypto_kx_SESSIONKEYBYTES],
    unsigned char       tx[crypto_kx_SESSIONKEYBYTES],
    const unsigned char pk[crypto_kx_PUBLICKEYBYTES],
    const unsigned char sk[crypto_kx_SECRETKEYBYTES],
    const unsigned char other_pk[crypto_kx_PUBLICKEYBYTES]);

oidc_error_t ipc_cryptWrite(const int sock, const unsigned char* key,
                            const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  return ipc_vcryptWrite(sock, key, fmt, args);
}

oidc_error_t ipc_vcryptWrite(const int sock, const unsigned char* key,
                             const char* fmt, va_list args) {
  char* msg = oidc_vsprintf(fmt, args);
  if (msg == NULL) {
    return oidc_errno;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG,
         "Doing encrypted ipc write of %lu bytes: '%s'", strlen(msg), msg);
  char* encryptedMessage = encryptForIpc(msg, key);
  if (encryptedMessage == NULL) {
    return oidc_errno;
  }
  oidc_error_t e = ipc_write(sock, encryptedMessage);
  secFree(encryptedMessage);
  return e;
}

void secFreeIpcKeySet(struct ipc_keySet* k) { secFree(k); }
void secFreePubSecKeySet(struct pubsec_keySet* k) { secFree(k); }
// TODO check the usage of secFree an replace them with theses functions

struct pubsec_keySet* generatePubSecKeys() {
  struct pubsec_keySet* keys = secAlloc(sizeof(struct pubsec_keySet));
  crypto_kx_keypair(keys->pub, keys->sec);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Generated pub/sec keys");
  return keys;
}

char* communicatePublicKey(const int                   _sock,
                           const struct pubsec_keySet* key_set) {
  if (key_set == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  char* pk_base64 = toBase64((char*)key_set->pub, crypto_kx_PUBLICKEYBYTES);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Communicating pub key");
  char* res = ipc_communicateWithSock(_sock, pk_base64);
  secFree(pk_base64);
  return res;
}

struct ipc_keySet* generateIpcKeys(const struct pubsec_keySet* pubsec_keys,
                                   const unsigned char*        partyPubKey,
                                   const int                   isServer) {
  if (pubsec_keys == NULL || partyPubKey == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  struct ipc_keySet*     keys = secAlloc(sizeof(struct ipc_keySet));
  crypto_kx_session_keys sessionKeys =
      isServer ? crypto_kx_server_session_keys : crypto_kx_client_session_keys;
  if (sessionKeys(keys->key_rx, keys->key_tx, pubsec_keys->pub,
                  pubsec_keys->sec, partyPubKey) != 0) {
    /* Suspicious party public key, bail out */
    oidc_errno = OIDC_ECRYPPUB;
    secFree(keys);
    return NULL;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Successfully generated session keys");
  return keys;
}

struct ipc_keySet* generateClientIpcKeys(
    const struct pubsec_keySet* pubsec_keys,
    const unsigned char*        serverPubKey) {
  return generateIpcKeys(pubsec_keys, serverPubKey, 0);
}

struct ipc_keySet* generateServerIpcKeys(
    const struct pubsec_keySet* pubsec_keys,
    const unsigned char*        clientPubKey) {
  return generateIpcKeys(pubsec_keys, clientPubKey, 1);
}

list_t* encryptionKeys = NULL;

char* server_ipc_cryptRead(const int sock, const char* client_pk_base64) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Doing encrypted ipc read");
  unsigned char client_pk[crypto_kx_PUBLICKEYBYTES];
  fromBase64(client_pk_base64, crypto_kx_PUBLICKEYBYTES, client_pk);
  struct pubsec_keySet* pubsec_keys = generatePubSecKeys();
  struct ipc_keySet* ipc_keys = generateServerIpcKeys(pubsec_keys, client_pk);
  if (ipc_keys == NULL) {
    secFree(pubsec_keys);
    return NULL;
  }
  char* encrypted_request = communicatePublicKey(sock, pubsec_keys);
  secFree(pubsec_keys);
  if (encrypted_request == NULL) {
    secFree(ipc_keys);
    return NULL;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Received encrypted request");
  char* decryptedRequest = decryptForIpc(encrypted_request, ipc_keys->key_rx);
  secFree(encrypted_request);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Decrypted request is '%s'",
         decryptedRequest);
  moresecure_memzero(ipc_keys->key_rx, crypto_kx_SESSIONKEYBYTES);
  if (decryptedRequest != NULL) {
    if (encryptionKeys == NULL) {
      encryptionKeys = list_new();
    }
    list_rpush(encryptionKeys, list_node_new(ipc_keys));
  } else {
    secFree(ipc_keys);
  }
  return decryptedRequest;
}

struct ipc_keySet* client_keyExchange(const int sock) {
  struct pubsec_keySet* pubsec_keys = generatePubSecKeys();
  char* server_pk_base64            = communicatePublicKey(sock, pubsec_keys);
  if (server_pk_base64 == NULL) {
    secFree(pubsec_keys);
    return NULL;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Received server public key");
  unsigned char server_pk[crypto_kx_PUBLICKEYBYTES];
  fromBase64(server_pk_base64, crypto_kx_PUBLICKEYBYTES, server_pk);
  secFree(server_pk_base64);
  struct ipc_keySet* ipc_keys = generateClientIpcKeys(pubsec_keys, server_pk);
  secFree(pubsec_keys);
  if (ipc_keys == NULL) {
    return NULL;
  }
  return ipc_keys;
}
