#ifndef PASSWORD_CRYPT_H
#define PASSWORD_CRYPT_H

void  initPasswordCrypt();
char* encryptPassword(const char* password, const char* salt);
char* decryptPassword(const char* cypher, const char* salt);

#endif  // PASSWORD_CRYPT_H
