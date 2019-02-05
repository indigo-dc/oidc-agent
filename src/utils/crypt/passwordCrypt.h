#ifndef PASSWORD_CRYPT_H
#define PASSWORD_CRYPT_H

void  initPasswordCrypt();
char* encryptPassword(const char* password);
char* decryptPassword(const char* cypher);

#endif  // PASSWORD_CRYPT_H
