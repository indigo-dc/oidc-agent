#ifndef OIDC_SETTINGS_H
#define OIDC_SETTINGS_H

#include "oidc_utilities.h"

// env var names
#define OIDC_SOCK_ENV_NAME "OIDC_SOCK"
#define OIDC_PID_ENV_NAME "OIDCD_PID"

#define DEFAULT_SCOPE "openid profile offline_access"


// file names
#define ISSUER_CONFIG_FILENAME "issuer.config"
#define ETC_ISSUER_CONFIG_FILE "/etc/oidc-agent/" ISSUER_CONFIG_FILENAME

#define MAX_PASS_TRIES 3
#define MAX_POLL 10
#define DELTA_POLL 1000 //milliseconds

// Colors
#ifdef NO_COLOR
#define C_ERROR C_RESET
#define C_PROMPT C_RESET
#define C_IMPORTANT C_RESET
#else
#ifndef C_ERROR
#define C_ERROR C_RED
#endif
#ifndef C_PROMPT
#define C_PROMPT C_CYN
#endif
#ifndef C_IMPORTANT
#define C_IMPORTANT C_YEL
#endif
#endif //NO_COLOR

#define CONF_ENDPOINT_SUFFIX ".well-known/openid-configuration"

static char* possibleCertFiles[] = {
  "/etc/ssl/certs/ca-certificates.crt", // Debian/Ubuntu/Gentoo etc.
  "/etc/pki/tls/certs/ca-bundle.crt",   // Fedora/RHEL
  "/etc/ssl/ca-bundle.pem",             // OpenSUSE
  "/etc/pki/tls/cacert.pem"             // OpenELEC
};


#endif // OIDC_SETTINGS_H
