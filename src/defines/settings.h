#ifndef OIDC_SETTINGS_H
#define OIDC_SETTINGS_H

// env var names
/**
 * the name of the environment variable used to locate the IPC socket
 */
#define OIDC_SOCK_ENV_NAME "OIDC_SOCK"
/**
 * the name of the environment variable used to locate the remote TCP socket
 */
#define OIDC_REMOTE_SOCK_ENV_NAME "OIDC_REMOTE_SOCK"
/**
 * the name of the environment variable that holds the agent pid
 */
#define OIDC_PID_ENV_NAME "OIDCD_PID"
/**
 * the name of the environment variable that might hold the oidcagentdir
 * location
 */
#define OIDC_CONFIG_DIR_ENV_NAME "OIDC_CONFIG_DIR"
/**
 * the scope used as default value
 */
#define DEFAULT_SCOPE "openid profile offline_access"

// Default env var names for arguments
#define OIDC_REFRESHTOKEN_ENV_NAME "OIDC_REFRESH_TOKEN"
#define OIDC_PASSWORD_ENV_NAME "OIDC_ENCRYPTION_PW"

// file names
/**
 * the path to the config dir, if not provided via make
 */
#ifndef CONFIG_PATH
#define CONFIG_PATH "/etc"
#endif
#define ISSUER_CONFIG_FILENAME "issuer.config"
#define ETC_ISSUER_CONFIG_FILE CONFIG_PATH "/oidc-agent/" ISSUER_CONFIG_FILENAME
#define PRIVILEGES_PATH CONFIG_PATH "/oidc-agent/privileges"
#define PUBCLIENTS_FILENAME "pubclients.config"
#define ETC_PUBCLIENTS_CONFIG_FILE \
  CONFIG_PATH "/oidc-agent/" PUBCLIENTS_FILENAME

#define MAX_PASS_TRIES 3
/**
 * maximum number of polling tries
 */
#define MAX_POLL 20
/**
 * the delta between two pollings in seconds
 */
#define DELTA_POLL 2  // seconds

#define HTTP_DEFAULT_PORT 4242
#define HTTP_FALLBACK_PORT 8080

#define CONF_ENDPOINT_SUFFIX ".well-known/openid-configuration"

extern char* possibleCertFiles[4];

/**
 * prefix for tmp-files generated during account generation;
 * if dynamic client registration is used, the client config is temporarily
 * saved in a file prefixed with that string
 */
#define CLIENT_TMP_PREFIX "/tmp/oidc-gen:"

#define AGENTDIR_LOCATION_CONFIG "~/.config/oidc-agent/"
#define AGENTDIR_LOCATION_DOT "~/.oidc-agent/"

#ifdef __linux__
#define URL_OPENER "xdg-open"
#elif __APPLE__
#define URL_OPENER "open"
#endif

#endif  // OIDC_SETTINGS_H
