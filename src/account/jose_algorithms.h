#ifndef JOSE_ALGORITHMS_H
#define JOSE_ALGORITHMS_H

struct jose_algorithms {
  char* id_token_signing_alg;
  char* id_token_encryption_alg;
  char* id_token_encryption_enc;

  char* userinfo_signing_alg;
  char* userinfo_encryption_alg;
  char* userinfo_encryption_enc;

  char* request_object_signing_alg;
  char* request_object_encryption_alg;
  char* request_object_encryption_enc;
};

struct jose_algorithms* createJoseAlgorithms(const char*, const char*,
                                             const char*, const char*,
                                             const char*, const char*,
                                             const char*, const char*,
                                             const char*);
void                    _secFreeJoseAlgorithms(struct jose_algorithms* algos);

#endif  // JOSE_ALGORITHMS_H
