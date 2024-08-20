#include "ipcCryptUtils.h"

#include <string.h>

#include "utils/crypt/crypt.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/string/stringUtils.h"

char* encryptForIpc(const char* msg, const unsigned char* key) {
  struct encryptionInfo* cryptResult =
      crypt_encryptWithKey((unsigned char*)msg, key);
  if (cryptResult->encrypted_base64 == NULL) {
    secFreeEncryptionInfo(cryptResult);
    return NULL;
  }
  char* encoded =
      oidc_sprintf("%lu:%s:%s", strlen(msg), cryptResult->nonce_base64,
                   cryptResult->encrypted_base64);
  secFreeEncryptionInfo(cryptResult);
  return encoded;
}

char* decryptForIpc(const char* msg, const unsigned char* key) {
  if (msg == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  char*  msg_tmp          = oidc_strcopy(msg);
  char*  len_str          = strtok(msg_tmp, ":");
  char*  nonce_base64     = strtok(NULL, ":");
  char*  encrypted_base64 = strtok(NULL, ":");
  size_t msg_len          = strToULong(len_str);
  if (nonce_base64 == NULL || encrypted_base64 == NULL) {
    secFree(msg_tmp);
    oidc_errno = OIDC_ECRYPMIPC;
    return NULL;
  }
  struct encryptionInfo crypt = {.nonce_base64     = nonce_base64,
                                 .encrypted_base64 = encrypted_base64,
                                 .cryptParameter   = newCryptParameters()};
  unsigned char*        decryptedMsg =
      crypt_decryptWithKey(&crypt, msg_len + crypt.cryptParameter.mac_len, key);
  secFree(msg_tmp);
  return (char*)decryptedMsg;
}
