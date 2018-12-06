#include "versionUtils.h"
#include "utils/oidc_error.h"

#include <stdio.h>
#include <syslog.h>

int versionAtLeast(const char* version, const char* minVersion) {
  if (version == NULL || strlen(version) < 5) {
    return 0;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "checking if version %s is at least %s",
         version, minVersion);
  unsigned short v_maj, v_min, v_pat = 0;
  unsigned short m_maj, m_min, m_pat = 0;

  sscanf(version, "%hu.%hu.%hu", &v_maj, &v_min, &v_pat);
  sscanf(minVersion, "%hu.%hu.%hu", &m_maj, &m_min, &m_pat);

  if (v_maj > m_maj) {
    return 1;
  }
  if (v_maj < m_maj) {
    return 0;
  }
  if (v_min > m_min) {
    return 1;
  }
  if (v_min < m_min) {
    return 0;
  }
  if (v_pat >= m_pat) {
    return 1;
  }
  return 0;
}

#define VERSION_LINE_FMT "Generated using version: "

char* versionLineToSimpleVersion(const char* version_line) {
  if (version_line == NULL) {
    return NULL;
  }
  char* tmp      = oidc_strcopy(version_line);
  char* location = strtok(tmp, VERSION_LINE_FMT);
  char* version  = oidc_strcopy(location);
  secFree(tmp);
  return version;
}

char* simpleVersionToVersionLine(const char* version) {
  return oidc_sprintf("%s%s", VERSION_LINE_FMT, version);
}
