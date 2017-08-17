#ifndef OIDC_ERROR_H
#define OIDC_ERROR_H

#include <string.h>

enum _oidc_error {
  OIDC_SUCCESS  = 0,
  OIDC_EERROR   = -1,
  OIDC_EALLOC   = -2,
  OIDC_EMEM     = -3,

  OIDC_EEOF     = -5,
  OIDC_EFOPEN   = -6,
  OIDC_EFREAD   = -7,
  OIDC_EWRITE     = -8,

  OIDC_EPASS    = -18,
  OIDC_ECRYPM   = -19,

  OIDC_EURL     = -10,
  OIDC_ESSL     = -11,
  OIDC_ECURLI   = -12,

  OIDC_EARGNULL = -20,

  OIDC_EJSONPARS = -30,
  OIDC_EJSONOBJ = -31,
  OIDC_EJSONNOFOUND = -32,

  OIDC_ETCS     = -40,
  OIDC_EIN      = -41,

  OIDC_EBADCONFIG = -50,
  OIDC_EOIDC      = -51,
  OIDC_ECRED      = -52,
  OIDC_ENOREFRSH   = -53,

  OIDC_EMKTMP     = -60,
  OIDC_EENVVAR    = -61,
  OIDC_EBIND      = -62,
  OIDC_ECONSOCK   = -63,
  OIDC_ECRSOCK    = -64,
  OIDC_ESOCKINV   = -65,
  OIDC_EIPCDIS    = -66,

  OIDC_ESELECT    = -68,
  OIDC_EIOCTL     = -69,
};

typedef enum _oidc_error oidc_error_t;

int oidc_errno;
char oidc_error[256];

static inline void oidc_seterror(char* error) {
  strncpy(oidc_error, error, sizeof(oidc_error)-1);
  oidc_error[sizeof(oidc_error)-1]='\0';
}

static inline char* oidc_perror() {
  switch(oidc_errno) {
    case OIDC_SUCCESS: return "success";
    case OIDC_EERROR: return "error";
    case OIDC_EALLOC: return "memory alloc failed";
    case OIDC_EMEM: return "system out of memory";
    case OIDC_EEOF: return "empty file";
    case OIDC_EFOPEN: return "could not open file";
    case OIDC_EFREAD: return "could not read file";
    case OIDC_EWRITE: return "could not write";
    case OIDC_EPASS: return "wrong password";
    case OIDC_ECRYPM: return "encryption malformed";
    case OIDC_EURL: return "could not connect to url";
    case OIDC_ESSL: return "error with ssl cert";
    case OIDC_ECURLI: return "could not init curl";
    case OIDC_EARGNULL: return "argument is NULL";
    case OIDC_EJSONPARS: return "could not parse json";
    case OIDC_EJSONOBJ: return "is not a json object";
    case OIDC_EJSONNOFOUND: return "could not find key";
    case OIDC_ETCS: return "error tcsetattr";
    case OIDC_EIN: return "error getline";
    case OIDC_EBADCONFIG: return "bad configuration";
    case OIDC_EOIDC: return oidc_error;
    case OIDC_ECRED: return "Bad credentials";
    case OIDC_ENOREFRSH: return "No refresh token";
    case OIDC_EMKTMP: return "Could not make temp socket directory";
    case OIDC_EENVVAR: return "Env var not set";
    case OIDC_EBIND: return "Could not bind socket";
    case OIDC_ECONSOCK: return "Could not connect to socket";
    case OIDC_ECRSOCK: return "Could not create socket";
    case OIDC_ESOCKINV: return "Invalid socket";
    case OIDC_EIOCTL: return "error ioctl";
    case OIDC_EIPCDIS: return "the other party disconnected";
    case OIDC_ESELECT: return "error select";
    default: return "Unknown error";
  }
}


#endif // OIDC_ERROR_H
