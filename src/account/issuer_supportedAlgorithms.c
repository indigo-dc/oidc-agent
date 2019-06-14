#include "issuer_supportedAlgorithms.h"
#include "utils/json.h"
#include "utils/listUtils.h"
#include "utils/memory.h"

struct supported_algorithms* createSupportedAlgorithms(
    const char* id_token_signing_alg_values_supported,
    const char* id_token_encryption_alg_values_supported,
    const char* id_token_encryption_enc_values_supported,
    const char* userinfo_signing_alg_values_supported,
    const char* userinfo_encryption_alg_values_supported,
    const char* userinfo_encryption_enc_values_supported,
    const char* request_object_signing_alg_values_supported,
    const char* request_object_encryption_alg_values_supported,
    const char* request_object_encryption_enc_values_supported) {
  struct supported_algorithms* algos =
      secAlloc(sizeof(struct supported_algorithms));
  algos->id_token_signing_alg_values =
      JSONArrayStringToList(id_token_signing_alg_values_supported);
  algos->id_token_encryption_alg_values =
      JSONArrayStringToList(id_token_encryption_alg_values_supported);
  algos->id_token_encryption_enc_values =
      JSONArrayStringToList(id_token_encryption_enc_values_supported);
  algos->userinfo_signing_alg_values =
      JSONArrayStringToList(userinfo_signing_alg_values_supported);
  algos->userinfo_encryption_alg_values =
      JSONArrayStringToList(userinfo_encryption_alg_values_supported);
  algos->userinfo_encryption_enc_values =
      JSONArrayStringToList(userinfo_encryption_enc_values_supported);
  algos->request_object_signing_alg_values =
      JSONArrayStringToList(request_object_signing_alg_values_supported);
  algos->request_object_encryption_alg_values =
      JSONArrayStringToList(request_object_encryption_alg_values_supported);
  algos->request_object_encryption_enc_values =
      JSONArrayStringToList(request_object_encryption_enc_values_supported);
  return algos;
}

void _secFreeSupportedAlgorithms(struct supported_algorithms* algos) {
  if (algos == NULL) {
    return;
  }
  secFreeList(algos->id_token_signing_alg_values);
  secFreeList(algos->id_token_encryption_alg_values);
  secFreeList(algos->id_token_encryption_enc_values);
  secFreeList(algos->userinfo_signing_alg_values);
  secFreeList(algos->userinfo_encryption_alg_values);
  secFreeList(algos->userinfo_encryption_enc_values);
  secFreeList(algos->request_object_signing_alg_values);
  secFreeList(algos->request_object_encryption_alg_values);
  secFreeList(algos->request_object_encryption_enc_values);
  secFree(algos);
}
