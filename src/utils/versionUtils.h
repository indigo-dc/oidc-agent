#ifndef OIDC_VERSION_UTILS_H
#define OIDC_VERSION_UTILS_H

#define MIN_BASE64_VERSION "2.0.0"

int versionAtLeast(const char* version, const char* minVersion);

#endif  // OIDC_VERSION_UTILS_H
