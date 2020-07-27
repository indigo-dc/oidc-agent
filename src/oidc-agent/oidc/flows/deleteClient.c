#include "deleteClient.h"

#include "oidc-agent/http/http_ipc.h"
#include "utils/oidc_error.h"
#include "utils/parseJson.h"

oidc_error_t deleteClient(const char* configuration_endpoint,
                          const char* registration_access_token,
                          const char* cert_path) {
  char* res = httpsDELETE(configuration_endpoint, NULL, cert_path,
                          registration_access_token);
  if (res == NULL) {
    return oidc_errno;
  }
  char* error = parseForError(res);
  if (error != NULL) {
    oidc_seterror(error);
    oidc_errno = OIDC_EOIDC;
    secFree(error);
    return oidc_errno;
  }
  secFree(error);
  return OIDC_SUCCESS;
}
