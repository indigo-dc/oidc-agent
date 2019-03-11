#ifndef RESULT_WITH_ENCRYPTION_PASSWORD_H
#define RESULT_WITH_ENCRYPTION_PASSWORD_H

struct resultWithEncryptionPassword {
  void* result;
  char* password;
};

#define RESULT_WITH_PASSWORD_NULL \
  (struct resultWithEncryptionPassword) { .result = NULL, .password = NULL }

#endif  // RESULT_WITH_ENCRYPTION_PASSWORD_H
