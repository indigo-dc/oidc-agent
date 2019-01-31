#include "device_code.h"

#include "defines/oidc_values.h"
#include "utils/json.h"
#include "utils/stringUtils.h"

#include <syslog.h>

struct oidc_device_code* getDeviceCodeFromJSON(const char* json) {
  if (NULL == json) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  struct key_value pairs[6];
  pairs[0].key = OIDC_KEY_DEVICECODE;
  pairs[1].key = OIDC_KEY_USERCODE;
  pairs[2].key = OIDC_KEY_VERIFICATIONURI;
  pairs[3].key = OIDC_KEY_VERIFICATIONURI_COMPLETE;
  pairs[4].key = OIDC_KEY_EXPIRESIN;
  pairs[5].key = OIDC_KEY_INTERVAL;
  cJSON* cjson = stringToJson(json);
  if (getJSONValues(cjson, pairs, sizeof(pairs) / sizeof(pairs[0])) < 0) {
    syslog(LOG_AUTHPRIV | LOG_ERR, "Error while parsing json\n");
    secFreeJson(cjson);
    secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
    return NULL;
  }
  size_t expires_in = strValid(pairs[4].value) ? strToInt(pairs[4].value) : 0;
  size_t interval   = strValid(pairs[5].value) ? strToInt(pairs[5].value) : 5;
  secFree(pairs[4].value);
  secFree(pairs[5].value);
  char* verification_uri          = pairs[2].value;
  char* verification_uri_complete = pairs[3].value;
  if (verification_uri == NULL) {
    verification_uri = getJSONValue(cjson, GOOGLE_KEY_VERIFICATIONURI);
  }  // needed for the google device flow that is not conforming to the spec
     // draft
  if (verification_uri_complete == NULL) {
    verification_uri_complete =
        getJSONValue(cjson, GOOGLE_KEY_VERIFICATIONURI_COMPLETE);
  }  // needed for the google device flow that is not conforming to the spec
     // draft
  secFreeJson(cjson);
  struct oidc_device_code* code =
      oidc_device_new(pairs[0].value, pairs[1].value, verification_uri,
                      verification_uri_complete, expires_in, interval);
  return code;
}

char* deviceCodeToJSON(struct oidc_device_code c) {
  cJSON* cjson = generateJSONObject(
      OIDC_KEY_DEVICECODE, cJSON_String,
      strValid(oidc_device_getDeviceCode(c)) ? oidc_device_getDeviceCode(c)
                                             : "",
      OIDC_KEY_USERCODE, cJSON_String,
      strValid(oidc_device_getUserCode(c)) ? oidc_device_getUserCode(c) : "",
      OIDC_KEY_VERIFICATIONURI, cJSON_String,
      strValid(oidc_device_getVerificationUri(c))
          ? oidc_device_getVerificationUri(c)
          : "",
      OIDC_KEY_VERIFICATIONURI_COMPLETE, cJSON_String,
      strValid(oidc_device_getVerificationUriComplete(c))
          ? oidc_device_getVerificationUriComplete(c)
          : "",
      OIDC_KEY_EXPIRESIN, cJSON_Number, oidc_device_getExpiresIn(c),
      OIDC_KEY_INTERVAL, cJSON_Number, oidc_device_getInterval(c), NULL);
  char* json = jsonToString(cjson);
  secFreeJson(cjson);
  return json;
}

void printDeviceCode(struct oidc_device_code c, int printQR, int terminalQR) {
  printf("\nUsing a browser on another device, visit:\n%s\n\nAnd enter the "
         "code: %s\n",
         oidc_device_getVerificationUri(c), oidc_device_getUserCode(c));
  if (printQR) {
    char* fmt = terminalQR
                    ? "qrencode -t ASCII \"%s\" 2>/dev/null"
                    : "qrencode -o /tmp/oidc-agent-device \"%s\" >/dev/null "
                      "2>&1 && display /tmp/oidc-agent-device&>/dev/null 2>&1";
    char* cmd =
        oidc_sprintf(fmt, strValid(oidc_device_getVerificationUriComplete(c))
                              ? oidc_device_getVerificationUriComplete(c)
                              : oidc_device_getVerificationUri(c));
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "QRencode cmd: %s", cmd);
    system(cmd);
    secFree(cmd);
    // printQrCode(oidc_device_getVerificationUriComplete(c) ?:
    // oidc_device_getVerificationUri(c));
  }
}
