#ifndef OIDC_AGENT_GPG_H
#define OIDC_AGENT_GPG_H

char*         decryptPGPMessage(const char* gpg);
char*         decryptPGPFileContent(const char* content);
unsigned char isPGPMessage(const char* content);
unsigned char isPGPOIDCFile(const char* shortname);
char*         encryptPGPWithVersionLine(const char* text, const char* gpg_key);
char*         extractPGPKeyID(const char* text);
char*         extractPGPKeyIDFromOIDCFile(const char* shortname);

#endif  // OIDC_AGENT_GPG_H
