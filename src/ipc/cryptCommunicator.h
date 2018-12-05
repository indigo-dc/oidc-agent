#ifndef CRYPT_COMMUNICATOR_H
#define CRYPT_COMMUNICATOR_H

#include <stdarg.h>

#include <sodium.h>

struct ipc_keySet {
  unsigned char key_rx[crypto_kx_SESSIONKEYBYTES];
  unsigned char key_tx[crypto_kx_SESSIONKEYBYTES];
};

void               secFreeIpcKeySet(struct ipc_keySet* k);
char*              ipc_cryptCommunicate(char* fmt, ...);
char*              ipc_vcryptCommunicate(char* fmt, va_list args);
struct ipc_keySet* client_ipc_writeToSock(int sock, char* fmt, ...);
struct ipc_keySet* client_ipc_vwriteToSock(int sock, char* fmt, va_list args);

#endif  // CRYPT_COMMUNICATOR_H
