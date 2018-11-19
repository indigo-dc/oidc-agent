#ifndef OIDC_SETTINGS_H
#define OIDC_SETTINGS_H

// env var names
#define OIDC_SOCK_ENV_NAME "OIDC_SOCK"
#define OIDC_PID_ENV_NAME "OIDCD_PID"

#define DEFAULT_SCOPE "openid profile offline_access"

// file names
#ifndef CONFIG_PATH
#define CONFIG_PATH "/etc"
#endif
#define ISSUER_CONFIG_FILENAME "issuer.config"
#define ETC_ISSUER_CONFIG_FILE CONFIG_PATH "/oidc-agent/" ISSUER_CONFIG_FILENAME
#define PRIVILEGES_PATH CONFIG_PATH "/oidc-agent/privileges"

#define MAX_PASS_TRIES 3
#define MAX_POLL 10
#define DELTA_POLL 1000  // milliseconds

#define CONF_ENDPOINT_SUFFIX ".well-known/openid-configuration"

static char* possibleCertFiles[] = {
    "/etc/ssl/certs/ca-certificates.crt",  // Debian/Ubuntu/Gentoo etc.
    "/etc/pki/tls/certs/ca-bundle.crt",    // Fedora/RHEL
    "/etc/ssl/ca-bundle.pem",              // OpenSUSE
    "/etc/pki/tls/cacert.pem"              // OpenELEC
};

#define CLIENT_TMP_PREFIX "/tmp/oidc-gen:"

#endif  // OIDC_SETTINGS_H
