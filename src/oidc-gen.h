#ifndef OIDC_GEN_H
#define OIDC_GEN_H

char* possibleCertFiles[] = {
"/etc/ssl/certs/ca-certificates.crt", // Debian/Ubuntu/Gentoo etc.
"/etc/pki/tls/certs/ca-bundle.crt",   // Fedora/RHEL
"/etc/ssl/ca-bundle.pem",             // OpenSUSE
"/etc/pki/tls/cacert.pem"             // OpenELEC
};

struct oidc_provider* genNewProvider();
char* encryptProvider(const char* json, const char* password) ;
void saveExit(int exitno);

#endif // OIDC_GEN_H
