#include "device_code.h"

#include "defines/mytoken_values.h"
#include "defines/oidc_values.h"
#include "utils/agentLogger.h"
#include "utils/json.h"
#include "utils/string/stringUtils.h"

struct oidc_device_code* getDeviceCodeFromJSON(const char* json) {
  if (NULL == json) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  INIT_KEY_VALUE(OIDC_KEY_DEVICECODE, OIDC_KEY_USERCODE,
                 OIDC_KEY_VERIFICATIONURI, OIDC_KEY_VERIFICATIONURI_COMPLETE,
                 OIDC_KEY_EXPIRESIN, OIDC_KEY_INTERVAL, MYTOKEN_KEY_CONSENTURI,
                 MYTOKEN_KEY_POLLINGCODE);
  cJSON* cjson = stringToJson(json);
  if (CALL_GETJSONVALUES_FROM_CJSON(cjson) < 0) {
    agent_log(ERROR, "Error while parsing json\n");
    secFreeJson(cjson);
    SEC_FREE_KEY_VALUES();
    return NULL;
  }
  KEY_VALUE_VARS(device_code, usercode, verification_uri,
                 verification_uri_complete, expires_in, interval, consent_uri,
                 polling_code);
  size_t expires_in = strToInt(_expires_in);
  size_t interval   = strValid(_interval) ? strToInt(_interval)
                                          : 5;  // Default of strToInt would be 0
  secFree(_expires_in);
  secFree(_interval);
  _verification_uri = _verification_uri ?: _consent_uri;
  _verification_uri_complete =
      _verification_uri_complete ?: oidc_strcopy(_consent_uri);
  if (_verification_uri == NULL) {
    _verification_uri = getJSONValue(cjson, GOOGLE_KEY_VERIFICATIONURI);
  }  // needed for the google device flow that is not conforming to the spec
  if (_verification_uri_complete == NULL) {
    _verification_uri_complete =
        getJSONValue(cjson, GOOGLE_KEY_VERIFICATIONURI_COMPLETE);
  }  // needed for the google device flow that is not conforming to the spec
  secFreeJson(cjson);
  if (_polling_code) {
    secFree(_device_code);
    _device_code = _polling_code;
  }
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
