#ifndef OIDC_ERROR_H
#define OIDC_ERROR_H

enum _oidc_error {
  OIDC_SUCCESS = 0,
  OIDC_EERROR  = -1,
  OIDC_EALLOC  = -2,
  OIDC_EMEM    = -3,

  OIDC_EEOF   = -5,
  OIDC_EFOPEN = -6,
  OIDC_EFREAD = -7,
  OIDC_EWRITE = -8,
  OIDC_EFNEX  = -9,

  OIDC_EURL   = -10,
  OIDC_ESSL   = -11,
  OIDC_ECURLI = -12,

  OIDC_ECRYPPUB  = -14,
  OIDC_EDECRYPT  = -15,
  OIDC_EENCRYPT  = -16,
  OIDC_ECRYPHASH = -17,
  OIDC_EPASS     = -18,
  OIDC_ECRYPM    = -19,
  OIDC_ECRYPMIPC = -190,

  OIDC_EARGNULL     = -20,
  OIDC_EARGNULLFUNC = -21,

  OIDC_EJSONPARS    = -30,
  OIDC_EJSONOBJ     = -31,
  OIDC_EJSONARR     = -32,
  OIDC_EJSONNOFOUND = -33,
  OIDC_EJSONADD     = -34,
  OIDC_EJSONMERGE   = -35,
  OIDC_EJSONTYPE    = -36,

  OIDC_ETCS = -40,
  OIDC_EIN  = -41,

  OIDC_EBADCONFIG = -50,
  OIDC_EOIDC      = -51,
  OIDC_ECRED      = -52,
  OIDC_ENOREFRSH  = -53,
  OIDC_ENODEVICE  = -54,
  OIDC_EFMT       = -55,
  OIDC_EUNSCOPE   = -56,
  OIDC_EPORTRANGE = -57,

  OIDC_EMKTMP   = -60,
  OIDC_EENVVAR  = -61,
  OIDC_EBIND    = -62,
  OIDC_ECONSOCK = -63,
  OIDC_ECRSOCK  = -64,
  OIDC_ESOCKINV = -65,
  OIDC_EIPCDIS  = -66,
  OIDC_EMSGSIZE = -67,
  OIDC_ESELECT  = -68,
  OIDC_EIOCTL   = -69,
  OIDC_ETIMEOUT = -600,
  OIDC_EGROUPNF = -601,

  OIDC_EMAXTRIES  = -70,
  OIDC_ENOACCOUNT = -71,

  OIDC_EHTTPD     = -81,
  OIDC_EHTTPPORTS = -80,
  OIDC_ENOREURI   = -82,
  OIDC_EHTTP0     = -83,

  OIDC_ENOSTATE    = -85,
  OIDC_ENOCODE     = -86,
  OIDC_ENOBASEURI  = -87,
  OIDC_EWRONGSTATE = -88,

  OIDC_ENOPRIVCONF = -90,

  OIDC_ENOSUPREG = -100,
  OIDC_ENOSUPREV = -101,

  OIDC_ENOPUBCLIENT = -106,

  OIDC_EPWNOTFOUND = -110,
  OIDC_EGERROR     = -111,
  OIDC_EUSRPWCNCL  = -112,
  OIDC_EFORBIDDEN  = -113,

  OIDC_ELOCKED    = -120,
  OIDC_ENOTLOCKED = -121,

  OIDC_EINTERNAL = -4242,

  OIDC_NOTIMPL = -1000,

  OIDC_ENOPE = -1337,
};

typedef enum _oidc_error oidc_error_t;

extern int  oidc_errno;
extern char oidc_error[1024];

void  oidc_seterror(const char* error);
void  oidc_setInternalError(const char* error);
void  oidc_setErrnoError();
void  oidc_setArgNullFuncError(const char* fncname);
char* oidc_serrorFor(oidc_error_t err);
char* oidc_serror();
int   errorMessageIsForError(const char* error_msg, oidc_error_t err);
void  oidc_perror();

#endif  // OIDC_ERROR_H
