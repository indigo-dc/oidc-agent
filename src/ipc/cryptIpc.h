#ifndef IPC_CRYPT_H
#define IPC_CRYPT_H

#include <sodium.h>
#include <stdarg.h>

#include "utils/oidc_error.h"

struct pubsec_keySet {
  unsigned char pk[crypto_kx_PUBLICKEYBYTES];
  unsigned char sk[crypto_kx_SECRETKEYBYTES];
};

char*          communicatePublicKey(const int _sock, const char* publicKey);
unsigned char* generateIpcKey(const unsigned char* publicKey,
                              const unsigned char* privateKey);
struct pubsec_keySet* generatePubSecKeys();
oidc_error_t ipc_cryptWrite(const int, const unsigned char*, const char*, ...);
oidc_error_t ipc_vcryptWrite(const int, const unsigned char*, const char*,
                             va_list);
void         secFreePubSecKeySet(struct pubsec_keySet*);
char*        server_ipc_cryptRead(const int, const char*);
unsigned char* client_keyExchange(const int sock);

#endif  // IPC_CRYPT_H
