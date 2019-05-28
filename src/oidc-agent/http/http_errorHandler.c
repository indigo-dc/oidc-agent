#include "http_errorHandler.h"
#include "utils/logger.h"
#include "utils/oidc_error.h"

oidc_error_t handleCURLE_OK(CURL* curl) {
  long http_code = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
  logger(DEBUG, "Received status code %ld", http_code);
  if (http_code >= 400) {
    oidc_errno = http_code;
  } else {
    oidc_errno = OIDC_SUCCESS;
  }
  return oidc_errno;
}

oidc_error_t handleSSL(int res, CURL* curl) {
  logger(ERROR,
         "%s (%s:%d) HTTPS Request failed: %s Please check the provided "
         "certh_path.\n",
         __func__, __FILE__, __LINE__, curl_easy_strerror(res));
  curl_easy_cleanup(curl);
  oidc_errno = OIDC_ESSL;
  return oidc_errno;
}

oidc_error_t handleHost(int res, CURL* curl) {
  logger(
      ERROR,
      "%s (%s:%d) HTTPS Request failed: %s Please check the provided URLs.\n",
      __func__, __FILE__, __LINE__, curl_easy_strerror(res));
  curl_easy_cleanup(curl);
  oidc_errno = OIDC_EURL;
  return oidc_errno;
}

oidc_error_t CURLErrorHandling(int res, CURL* curl) {
  switch (res) {
    case CURLE_OK: return handleCURLE_OK(curl);
    case CURLE_URL_MALFORMAT:
    case CURLE_COULDNT_RESOLVE_HOST: return handleHost(res, curl);
    case CURLE_SSL_CONNECT_ERROR:
    case CURLE_SSL_CERTPROBLEM:
    case CURLE_SSL_CIPHER:
    case CURLE_SSL_CACERT:
    case CURLE_SSL_CACERT_BADFILE:
    case CURLE_SSL_CRL_BADFILE:
    case CURLE_SSL_ISSUER_ERROR: return handleSSL(res, curl);
    default:
      logger(ERROR, "%s (%s:%d) curl_easy_perform() failed: %s\n", __func__,
             __FILE__, __LINE__, curl_easy_strerror(res));
      curl_easy_cleanup(curl);
      oidc_errno = OIDC_EERROR;
      return OIDC_EERROR;
  }
}
