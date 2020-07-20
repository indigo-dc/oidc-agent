#include "device_code.h"
#include "defines/oidc_values.h"
#include "utils/agentLogger.h"
#include "utils/json.h"
#include "utils/stringUtils.h"

struct oidc_device_code* getDeviceCodeFromJSON(const char* json) {
  if (NULL == json) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  INIT_KEY_VALUE(OIDC_KEY_DEVICECODE, OIDC_KEY_USERCODE,
                 OIDC_KEY_VERIFICATIONURI, OIDC_KEY_VERIFICATIONURI_COMPLETE,
                 OIDC_KEY_EXPIRESIN, OIDC_KEY_INTERVAL);
  cJSON* cjson = stringToJson(json);
  if (CALL_GETJSONVALUES_FROM_CJSON(cjson) < 0) {
    agent_log(ERROR, "Error while parsing json\n");
    secFreeJson(cjson);
    SEC_FREE_KEY_VALUES();
    return NULL;
  }
  KEY_VALUE_VARS(device_code, usercode, verification_uri,
                 verification_uri_complete, expires_in, interval);
  size_t expires_in = strToInt(_expires_in);
  size_t interval   = strValid(_interval) ? strToInt(_interval)
                                        : 5;  // Default of strToInt would be 0
  secFree(_expires_in);
  secFree(_interval);
  if (_verification_uri == NULL) {
    _verification_uri = getJSONValue(cjson, GOOGLE_KEY_VERIFICATIONURI);
  }  // needed for the google device flow that is not conforming to the spec
     // draft
  if (_verification_uri_complete == NULL) {
    _verification_uri_complete =
        getJSONValue(cjson, GOOGLE_KEY_VERIFICATIONURI_COMPLETE);
  }  // needed for the google device flow that is not conforming to the spec
     // draft
  secFreeJson(cjson);
  struct oidc_device_code* code =
      oidc_device_new(_device_code, _usercode, _verification_uri,
                      _verification_uri_complete, expires_in, interval);
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

void printDeviceCode(struct oidc_device_code c) {
  printNormal(
      "\nUsing a browser on another device, visit:\n%s\n\nAnd enter the "
      "code: %s\n",
      oidc_device_getVerificationUri(c), oidc_device_getUserCode(c));
  char* cmd = oidc_sprintf("qrencode -t UTF8 \"%s\" 2>/dev/null",
                           strValid(oidc_device_getVerificationUriComplete(c))
                               ? oidc_device_getVerificationUriComplete(c)
                               : oidc_device_getVerificationUri(c));
  if (system("qrencode --version") == 0) {  // Check if qrencode is installed
    printNormal("Alternatively you can use the following QR code to visit the "
                "above listed URL.");
    if (system(cmd) != 0) {
      logger(ERROR, "Cannot open QRencode");
    }
  }
  secFree(cmd);
}
