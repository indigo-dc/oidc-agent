#ifndef OIDC_VERSION_UTILS_H
#define OIDC_VERSION_UTILS_H

#define MIN_BASE64_VERSION "2.1.0"

int   versionAtLeast(const char* version, const char* minVersion);
char* versionLineToSimpleVersion(const char* version_line);
char* simpleVersionToVersionLine(const char* version);

#endif  // OIDC_VERSION_UTILS_H
