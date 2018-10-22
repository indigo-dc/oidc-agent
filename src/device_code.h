#ifndef DEVICE_CODE_H
#define DEVICE_CODE_H

#include "utils/memory.h"

#include <stddef.h>
#include <stdlib.h>

struct oidc_device_code {
  char*  device_code;
  char*  user_code;
  char*  verification_uri;
  char*  verification_uri_complete;
  size_t expires_in;
  size_t interval;
};

static inline char* oidc_device_getDeviceCode(struct oidc_device_code c) {
  return c.device_code;
}
static inline char* oidc_device_getUserCode(struct oidc_device_code c) {
  return c.user_code;
}
static inline char* oidc_device_getVerificationUri(struct oidc_device_code c) {
  return c.verification_uri;
}
static inline char* oidc_device_getVerificationUriComplete(
    struct oidc_device_code c) {
  return c.verification_uri_complete;
}
static inline size_t oidc_device_getExpiresIn(struct oidc_device_code c) {
  return c.expires_in;
}
static inline size_t oidc_device_getInterval(struct oidc_device_code c) {
  return c.interval;
}

static inline void oidc_device_setDeviceCode(struct oidc_device_code* c,
                                             char* device_code) {
  secFree(c->device_code);
  c->device_code = device_code;
}
static inline void oidc_device_setUserCode(struct oidc_device_code* c,
                                           char*                    user_code) {
  secFree(c->user_code);
  c->user_code = user_code;
}
static inline void oidc_device_setVerificationUrl(struct oidc_device_code* c,
                                                  char* verification_uri) {
  secFree(c->verification_uri);
  c->verification_uri = verification_uri;
}
static inline void oidc_device_setVerificationUrlComplete(
    struct oidc_device_code* c, char* verification_uri_complete) {
  secFree(c->verification_uri_complete);
  c->verification_uri_complete = verification_uri_complete;
}
static inline void oidc_device_setExpiresIn(struct oidc_device_code* c,
                                            size_t expires_in) {
  c->expires_in = expires_in;
}
static inline void oidc_device_setInterval(struct oidc_device_code* c,
                                           size_t                   interval) {
  c->interval = interval;
}

static inline struct oidc_device_code* oidc_device_new(
    char* device_code, char* user_code, char* verification_uri,
    char* verification_uri_complete, size_t expires_in, size_t interval) {
  struct oidc_device_code* c = secAlloc(sizeof(struct oidc_device_code));
  oidc_device_setDeviceCode(c, device_code);
  oidc_device_setUserCode(c, user_code);
  oidc_device_setVerificationUrl(c, verification_uri);
  oidc_device_setVerificationUrlComplete(c, verification_uri_complete);
  oidc_device_setExpiresIn(c, expires_in);
  oidc_device_setInterval(c, interval);
  return c;
}

static inline void _secFreeDeviceCode(struct oidc_device_code* c) {
  oidc_device_setDeviceCode(c, NULL);
  oidc_device_setUserCode(c, NULL);
  oidc_device_setVerificationUrl(c, NULL);
  oidc_device_setVerificationUrlComplete(c, NULL);
  oidc_device_setExpiresIn(c, 0);
  oidc_device_setInterval(c, 0);
  secFree(c);
}

struct oidc_device_code* getDeviceCodeFromJSON(char* json);
char*                    deviceCodeToJSON(struct oidc_device_code c);
void printDeviceCode(struct oidc_device_code c, int printQR, int terminalQR);

#ifndef secFreeDeviceCode
#define secFreeDeviceCode(ptr) \
  do {                         \
    _secFreeDeviceCode((ptr)); \
    (ptr) = NULL;              \
  } while (0)
#endif  // secFreeDeviceCode

#endif  // DEVICE_CODE_H
