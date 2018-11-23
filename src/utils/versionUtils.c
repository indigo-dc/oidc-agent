#include "versionUtils.h"

#include <stdio.h>

int versionAtLeast(const char* version, const char* minVersion) {
  if (version == NULL) {
    return 0;
  }
  unsigned short v_maj, v_min, v_pat;
  unsigned short m_maj, m_min, m_pat;

  sscanf("%us.%us.%us", version, &v_maj, &v_min, &v_pat);
  sscanf("%us.%us.%us", minVersion, &m_maj, &m_min, &m_pat);

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
  if (v_pat >= m_min) {
    return 1;
  }
  return 0;
}
