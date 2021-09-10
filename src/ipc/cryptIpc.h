#ifndef IPC_CRYPT_H
#define IPC_CRYPT_H

#include "utils/oidc_error.h"

#include <sodium.h>
#include <stdarg.h>
#ifdef __MSYS__
#include <winsock2.h>
#else
#include "socket.h"
#endif

struct pubsec_keySet {
  unsigned char pk[crypto_kx_PUBLICKEYBYTES];
  unsigned char sk[crypto_kx_SECRETKEYBYTES];
};

char*          communicatePublicKey(const SOCKET _sock, const char* publicKey);
unsigned char* generateIpcKey(const unsigned char* publicKey,
                              const unsigned char* privateKey);
struct pubsec_keySet* generatePubSecKeys();
oidc_error_t ipc_cryptWrite(const SOCKET, const unsigned char*, const char*, ...);
oidc_error_t ipc_vcryptWrite(const SOCKET, const unsigned char*, const char*,
                             va_list);
void         secFreePubSecKeySet(struct pubsec_keySet*);
char*        server_ipc_cryptRead(const SOCKET, const char*);
unsigned char* client_keyExchange(const SOCKET sock);

#endif  // IPC_CRYPT_H
