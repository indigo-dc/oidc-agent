#ifndef ISSUER_SUPPORTED_ALGORITHMS_H
#define ISSUER_SUPPORTED_ALGORITHMS_H

#include "list/list.h"

struct supported_algorithms {
  list_t* id_token_signing_alg_values;
  list_t* id_token_encryption_alg_values;
  list_t* id_token_encryption_enc_values;

  list_t* userinfo_signing_alg_values;
  list_t* userinfo_encryption_alg_values;
  list_t* userinfo_encryption_enc_values;

  list_t* request_object_signing_alg_values;
  list_t* request_object_encryption_alg_values;
  list_t* request_object_encryption_enc_values;
};

struct supported_algorithms* createSupportedAlgorithms(const char*, const char*,
                                                       const char*, const char*,
                                                       const char*, const char*,
                                                       const char*, const char*,
                                                       const char*);
void _secFreeSupportedAlgorithms(struct supported_algorithms* algos);

#endif  // ISSUER_SUPPORTED_ALGORITHMS_H
