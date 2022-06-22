#include "settings.h"

#include "defines/msys.h"
#ifdef ANY_MSYS
#include "utils/file_io/fileUtils.h"
#include "utils/string/stringUtils.h"

#ifndef CONFIG_PATH
#define CONFIG_PATH \
  "$ProgramData/oidc-agent"  // The full path has two / oidc-agent the second
                             // one is appended later
#endif
#define CERT_PATH CONFIG_PATH "/oidc-agent/ca-bundle.crt"

char* _cert_file                  = NULL;
char* _config_path                = NULL;
char* _etc_issuer_config_file     = NULL;
char* _etc_pubclients_config_file = NULL;

const char* CERT_FILE() {
  if (_cert_file == NULL) {
    _cert_file = fillEnvVarsInPath(CERT_PATH);
  }
  return _cert_file;
}

const char* ETC_ISSUER_CONFIG_FILE() {
  if (_config_path == NULL) {
    _config_path = fillEnvVarsInPath(CONFIG_PATH);
  }
  if (_etc_issuer_config_file == NULL) {
    _etc_issuer_config_file =
        oidc_pathcat(_config_path, "oidc-agent/" ISSUER_CONFIG_FILENAME);
  }
  return _etc_issuer_config_file;
}

const char* ETC_PUBCLIENTS_CONFIG_FILE() {
  if (_config_path == NULL) {
    _config_path = fillEnvVarsInPath(CONFIG_PATH);
  }
  if (_etc_pubclients_config_file == NULL) {
    _etc_pubclients_config_file =
        oidc_pathcat(_config_path, "oidc-agent/" PUBCLIENTS_FILENAME);
  }
  return _etc_pubclients_config_file;
}

#else

char* possibleCertFiles[4] = {
    "/etc/ssl/certs/ca-certificates.crt",  // Debian/Ubuntu/Gentoo etc.
    "/etc/pki/tls/certs/ca-bundle.crt",    // Fedora/RHEL
    "/etc/ssl/ca-bundle.pem",              // OpenSUSE
    "/etc/pki/tls/cacert.pem"              // OpenELEC
};
#endif
