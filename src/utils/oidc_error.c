#include "oidc_error.h"

#include <errno.h>
#include <string.h>

#include "utils/memory.h"
#include "utils/memzero.h"
#include "utils/printer.h"
#include "utils/string/stringUtils.h"

int  oidc_errno;
char oidc_error[1024];

void oidc_seterror(const char* error) {
  moresecure_memzero(oidc_error, sizeof(oidc_error));
  strncpy(oidc_error, error, sizeof(oidc_error) - 1);
  oidc_error[sizeof(oidc_error) - 1] = '\0';
}

void oidc_setInternalError(const char* error) {
  char error_msg[1024];
  strcpy(error_msg, "Internal error: ");
  strncpy(error_msg + strlen("Internal error: "), error,
          sizeof(error_msg) - strlen("Internal error: ") - 1);
  oidc_seterror(error_msg);
  oidc_errno = OIDC_EINTERNAL;
}

void oidc_setErrnoError() {
  oidc_errno = OIDC_EERROR;
  return oidc_seterror(strerror(errno));
}

void oidc_setArgNullFuncError(const char* fncname) {
  char* err = oidc_sprintf("Argument is NULL in function %s", fncname);
  oidc_seterror(err);
  secFree(err);
  oidc_errno = OIDC_EARGNULLFUNC;
}

char* oidc_serrorFor(oidc_error_t err) {
  switch (err) {
    case OIDC_SUCCESS: return "success";
    case OIDC_EERROR: return oidc_error;
    case OIDC_EALLOC: return "memory alloc failed";
    case OIDC_EMEM: return "system out of memory";
    case OIDC_EEOF: return "empty file";
    case OIDC_EFOPEN: return "could not open file";
    case OIDC_EFREAD: return "could not read file";
    case OIDC_EWRITE: return "could not write";
    case OIDC_EFNEX: return "could not open file - file does not exist";
    case OIDC_EPASS: return "wrong password";
    case OIDC_ECRYPPUB: return "received suspicious public key";
    case OIDC_ECRYPM: return "encryption malformed";
    case OIDC_ECRYPMIPC:
      return "internal error: ipc encrypted message malformed";
    case OIDC_EENCRYPT: return "encryption failed";
    case OIDC_EDECRYPT: return "decryption failed";
    case OIDC_ECRYPHASH: return "could not hash string";
    case OIDC_EURL: return "could not connect to url";
    case OIDC_ESSL: return "error with ssl cert";
    case OIDC_ECURLI: return "could not init curl";
    case OIDC_EARGNULL: return "argument is NULL";
    case OIDC_EARGNULLFUNC: return oidc_error;
    case OIDC_EJSONPARS: return "could not parse json";
    case OIDC_EJSONOBJ: return "is not a json object";
    case OIDC_EJSONARR: return "is not a json array";
    case OIDC_EJSONNOFOUND: return "could not find json key";
    case OIDC_EJSONADD: return "The json string does not end with '}'";
    case OIDC_EJSONMERGE: return "Cannot merge json objects";
    case OIDC_EJSONTYPE: return "Unknown cJSON Type";
    case OIDC_ETCS: return "error tcsetattr";
    case OIDC_EIN: return "error getline";
    case OIDC_EBADCONFIG: return "bad configuration";
    case OIDC_EOIDC: return oidc_error;
    case OIDC_ECRED: return "Bad credentials";
    case OIDC_ENOREFRSH: return "No or malformed refresh token";
    case OIDC_ENODEVICE: return "Device Flow not Supported by OpenID Provider";
    case OIDC_EFMT: return "Format Validation Error";
    case OIDC_EUNSCOPE:
      return "Could not register the necessary scopes dynamically";
    case OIDC_EPORTRANGE: return "Port not in valid range";
    case OIDC_EMKTMP: return "Could not make temp socket directory";
    case OIDC_EENVVAR: return oidc_error;
    case OIDC_EBIND: return "Could not bind ipc-socket";
    case OIDC_ECONSOCK: return "Could not connect to oidc-agent";
    case OIDC_ECRSOCK: return "Could not create ipc-socket";
    case OIDC_EMSGSIZE: return "Message size exceeds maximum package size";
    case OIDC_ESOCKINV: return "Invalid socket";
    case OIDC_EMSYSAUTH: return "Could not authorize against MSYS emualted socket";
    case OIDC_EIOCTL: return "error ioctl";
    case OIDC_EIPCDIS: return "the other party disconnected";
    case OIDC_ETIMEOUT: return "reached timeout";
    case OIDC_EGROUPNF: return "Group does not exist";
    case OIDC_ESELECT: return "error select";
    case OIDC_EMAXTRIES: return "reached maximum number of tries";
    case OIDC_ENOACCOUNT: return "No account configured with that short name";
    case OIDC_EHTTPD: return "Could not start http server";
    case OIDC_EHTTPPORTS:
      return "Could not start the http server on any of the registered "
             "redirect uris.";
    case OIDC_ENOREURI: return "No redirect_uri specified";
    case OIDC_EHTTP0: return "Internal error: Http sent 0";
    case OIDC_ENOSTATE: return "redirected uri did not contain state parameter";
    case OIDC_ENOCODE: return "redirected uri did not contain code parameter";
    case OIDC_ENOBASEURI: return "could not get base uri from redirected uri";
    case OIDC_EWRONGSTATE: return "wrong state";
    case OIDC_EWRONGDEVICECODE: return "wrong device code";
    case OIDC_ENOSUPREG:
      return "Dynamic registration is not supported by this issuer. Please "
             "register a client manually and then run oidc-gen with the -m "
             "flag.";
    case OIDC_ENOSUPREV:
      return "Token revocation is not supported by this issuer.";
    case OIDC_ENOPUBCLIENT: return "No public client found for this issuer";
    case OIDC_ELOCKED: return "Agent locked";
    case OIDC_ENOTLOCKED: return "Agent not locked";
    case OIDC_EINTERNAL: return oidc_error;
    case OIDC_EPWNOTFOUND: return "Password not found";
    case OIDC_EGERROR: return oidc_error;
    case OIDC_EUSRPWCNCL: return "user cancelled password prompt";
    case OIDC_EFORBIDDEN: return "operation forbidden";
    case OIDC_NOTIMPL: return "Not yet implemented";
    case OIDC_ENOPE: return "Computer says NO!";
    default: return "Computer says NO!";
  }
}

char* oidc_serror() {
  if (oidc_errno >= 200 && oidc_errno < 600) {
    char* error = oidc_sprintf("Received Http Status Code %d", oidc_errno);
    oidc_seterror(error);
    secFree(error);
    return oidc_error;
  }
  return oidc_serrorFor(oidc_errno);
}

int errorMessageIsForError(const char* error_msg, oidc_error_t err) {
  if (error_msg == NULL) {
    return 0;
  }
  return strequal(error_msg, oidc_serrorFor(err));
}

void oidc_perror() { printError("Error: %s\n", oidc_serror()); }

struct oidc_error_state* saveErrorState() {
  struct oidc_error_state* state = secAlloc(sizeof(struct oidc_error_state));
  state->oidc_errno              = oidc_errno;
  state->oidc_error              = oidc_strcopy(oidc_error);
  return state;
}

void restoreErrorState(struct oidc_error_state* state) {
  if (state == NULL) {
    return;
  }
  oidc_errno = state->oidc_errno;
  oidc_seterror(state->oidc_error);
}

void restoreAndFreeErrorState(struct oidc_error_state* state) {
  restoreErrorState(state);
  secFreeErrorState(state);
}

void secFreeErrorState(struct oidc_error_state* state) {
  if (state == NULL) {
    return;
  }
  secFree(state->oidc_error);
  secFree(state);
}
