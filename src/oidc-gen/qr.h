#ifndef OIDC_AGENT_QR_H
#define OIDC_AGENT_QR_H

char* getQRCode(const char* content);
char* getUTF8QRCode(const char* content);
char* getANSIQRCode(const char* content);

#endif  // OIDC_AGENT_QR_H
