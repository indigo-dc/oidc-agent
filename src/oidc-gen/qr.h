#ifndef OIDC_AGENT_QR_H
#define OIDC_AGENT_QR_H

char* getQRCode(const char* content);
char* getUTF8QRCode(const char* content);
char* getANSIQRCode(const char* content);
int   getIMGQRCode(const char* content, const char* outpath);

#endif  // OIDC_AGENT_QR_H
