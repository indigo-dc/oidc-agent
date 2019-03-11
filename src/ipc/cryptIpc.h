#ifndef IPC_CRYPT_H
#define IPC_CRYPT_H

#include "utils/oidc_error.h"

#include <sodium.h>
#include <stdarg.h>

struct ipc_keySet {
  unsigned char key_rx[crypto_kx_SESSIONKEYBYTES];
  unsigned char key_tx[crypto_kx_SESSIONKEYBYTES];
};
struct pubsec_keySet {
  unsigned char pub[crypto_kx_PUBLICKEYBYTES];
  unsigned char sec[crypto_kx_SECRETKEYBYTES];
};

char*              communicatePublicKey(const int, const struct pubsec_keySet*);
struct ipc_keySet* generateServerIpcKeys(const struct pubsec_keySet*,
                                         const unsigned char*);
struct ipc_keySet* generateClientIpcKeys(const struct pubsec_keySet*,
                                         const unsigned char*);
struct pubsec_keySet* generatePubSecKeys();
oidc_error_t ipc_cryptWrite(const int, const unsigned char*, const char*, ...);
oidc_error_t ipc_vcryptWrite(const int, const unsigned char*, const char*,
                             va_list);
void         secFreeIpcKeySet(struct ipc_keySet*);
void         secFreePubSecKeySet(struct pubsec_keySet*);
char*        server_ipc_cryptRead(const int, const char*);
struct ipc_keySet* client_keyExchange(const int sock);

#endif  // IPC_CRYPT_H
