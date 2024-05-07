#include "settings.h"

#include "defines/msys.h"
#ifdef ANY_MSYS
#include "utils/file_io/fileUtils.h"
#include "utils/logger.h"
#include "utils/string/stringUtils.h"

#ifndef CONFIG_PATH
#define CONFIG_PATH \
  "$ProgramData/oidc-agent"  // The full path has two / oidc-agent the second
                             // one is appended later
#endif

char* _config_path               = NULL;
char* _cert_file                 = NULL;
char* _etc_issuer_config_file    = NULL;
char* _etc_issuer_config_dir     = NULL;
char* _etc_custom_parameter_file = NULL;
char* _etc_config_file           = NULL;
char* _etc_mytoken_base          = NULL;

static const char* config_path() {
  if (_config_path == NULL) {
    _config_path = fillEnvVarsInPath(CONFIG_PATH);
  }
  return _config_path;
}

const char* CERT_FILE() {
  if (_cert_file == NULL) {
    _cert_file = oidc_pathcat(config_path(), "oidc-agent/ca-bundle.crt");
  }
  return _cert_file;
}

const char* ETC_ISSUER_CONFIG_FILE() {
  if (_etc_issuer_config_file == NULL) {
    _etc_issuer_config_file =
        oidc_pathcat(config_path(), "oidc-agent/" ISSUER_CONFIG_FILENAME);
  }
  return _etc_issuer_config_file;
}

const char* ETC_ISSUER_CONFIG_DIR() {
  if (_etc_issuer_config_dir == NULL) {
    _etc_issuer_config_dir =
        oidc_pathcat(config_path(), "oidc-agent/" ISSUER_CONFIG_DIRNAME);
  }
  return _etc_issuer_config_dir;
}

const char* ETC_CUSTOM_PARAMETERS_FILE() {
  if (_etc_custom_parameter_file == NULL) {
    _etc_custom_parameter_file =
        oidc_pathcat(config_path(), "oidc-agent/" CUSTOM_PARAMETERS_FILENAME);
  }
  return _etc_custom_parameter_file;
}

const char* ETC_CONFIG_FILE() {
  if (_etc_config_file == NULL) {
    _etc_config_file = oidc_pathcat(config_path(), "oidc-agent/config");
  }
  return _etc_config_file;
}

const char* _MYTOKEN_GLOBAL_BASE() {
  if (_etc_mytoken_base == NULL) {
    _etc_mytoken_base = oidc_pathcat(config_path(), "mytoken");
  }
  return _etc_mytoken_base;
}

#else

char* possibleCertFiles[6] = {
    "/etc/ssl/certs/ca-certificates.crt",  // Debian/Ubuntu/Gentoo etc.
    "/etc/pki/tls/certs/ca-bundle.crt",    // Fedora/RHEL
    "/etc/ssl/ca-bundle.pem",              // OpenSUSE
    "/etc/pki/tls/cacert.pem",             // OpenELEC
    "/etc/ssl/cert.pem",
    CONFIG_PATH "/ca-certificates/cert.pem"};
#endif
