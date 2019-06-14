#include "jose_algorithms.h"
#include "utils/memory.h"
#include "utils/stringUtils.h"

struct jose_algorithms* createJoseAlgorithms(
    const char* id_token_signing_alg, const char* id_token_encryption_alg,
    const char* id_token_encryption_enc, const char* userinfo_signing_alg,
    const char* userinfo_encryption_alg, const char* userinfo_encryption_enc,
    const char* request_object_signing_alg,
    const char* request_object_encryption_alg,
    const char* request_object_encryption_enc) {
  struct jose_algorithms* algos     = secAlloc(sizeof(struct jose_algorithms));
  algos->id_token_signing_alg       = oidc_strcopy(id_token_signing_alg);
  algos->id_token_encryption_alg    = oidc_strcopy(id_token_encryption_alg);
  algos->id_token_encryption_enc    = oidc_strcopy(id_token_encryption_enc);
  algos->userinfo_signing_alg       = oidc_strcopy(userinfo_signing_alg);
  algos->userinfo_encryption_alg    = oidc_strcopy(userinfo_encryption_alg);
  algos->userinfo_encryption_enc    = oidc_strcopy(userinfo_encryption_enc);
  algos->request_object_signing_alg = oidc_strcopy(request_object_signing_alg);
  algos->request_object_encryption_alg =
      oidc_strcopy(request_object_encryption_alg);
  algos->request_object_encryption_enc =
      oidc_strcopy(request_object_encryption_enc);
  return algos;
}

void _secFreeJoseAlgorithms(struct jose_algorithms* algos) {
  if (algos == NULL) {
    return;
  }
  secFree(algos->id_token_signing_alg);
  secFree(algos->id_token_encryption_alg);
  secFree(algos->id_token_encryption_enc);
  secFree(algos->userinfo_signing_alg);
  secFree(algos->userinfo_encryption_alg);
  secFree(algos->userinfo_encryption_enc);
  secFree(algos->request_object_signing_alg);
  secFree(algos->request_object_encryption_alg);
  secFree(algos->request_object_encryption_enc);
  secFree(algos);
}
