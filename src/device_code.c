#include "device_code.h"

#include "json.h"
#include "utils/stringUtils.h"

#include <syslog.h>

struct oidc_device_code* getDeviceCodeFromJSON(char* json) {
  if(NULL==json) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  struct key_value pairs[6];
  pairs[0].key = "device_code";
  pairs[1].key = "user_code";
  pairs[2].key = "verification_uri";
  pairs[3].key = "verification_uri_complete";
  pairs[4].key = "expires_in";
  pairs[5].key = "interval";
  if(getJSONValues(json, pairs, sizeof(pairs)/sizeof(pairs[0]))<0) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "Error while parsing json\n");
    return NULL;
  }
  size_t expires_in = strValid(pairs[4].value) ? atoi(pairs[4].value) : 0;
  size_t interval = strValid(pairs[5].value) ? atoi(pairs[5].value) : 5;
  clearFreeString(pairs[4].value);
  clearFreeString(pairs[5].value);
  char* verification_uri = pairs[2].value;
  char* verification_uri_complete = pairs[3].value;
  if(!isValid(verification_uri)) { verification_uri = getJSONValue(json, "verification_url"); } // needed for the google device flow that is not conforming to the spec draft
  if(!isValid(verification_uri_complete)) { verification_uri_complete = getJSONValue(json, "verification_url_complete"); } // needed for the google device flow that is not conforming to the spec draft
  struct oidc_device_code* code = oidc_device_new(pairs[0].value, pairs[1].value, verification_uri, verification_uri_complete, expires_in, interval);
  return code;
}
char* deviceCodeToJSON(struct oidc_device_code c) {
  char* fmt = "{\n\"device_code\":\"%s\",\n\"user_code\":\"%s\",\n\"verification_uri\":\"%s\",\n\"verification_uri_complete\":\"%s\",\n\"expires_in\":%lu,\n\"interval\":%lu\n}";
  char* c_json = oidc_sprintf(fmt, 
      strValid(oidc_device_getDeviceCode(c)) ? oidc_device_getDeviceCode(c) : "", 
      strValid(oidc_device_getUserCode(c)) ? oidc_device_getUserCode(c) : "", 
      strValid(oidc_device_getVerificationUri(c)) ? oidc_device_getVerificationUri(c) : "", 
      strValid(oidc_device_getVerificationUriComplete(c)) ? oidc_device_getVerificationUriComplete(c) : "", 
      oidc_device_getExpiresIn(c), 
      oidc_device_getInterval(c)
      );
  return c_json;
}

void printDeviceCode(struct oidc_device_code c, int printQR) {
  printf("\nUsing a browser on another device, visit:\n%s\n\nAnd enter the code: %s\n", oidc_device_getVerificationUri(c), oidc_device_getUserCode(c));
  if(printQR) {
    char* fmt = "qrencode -o /tmp/oidc-agent-device \"%s\" >/dev/null 2>&1 && display /tmp/oidc-agent-device&>/dev/null 2>&1";
    char* cmd = oidc_sprintf(fmt, strValid(oidc_device_getVerificationUriComplete(c)) ? oidc_device_getVerificationUriComplete(c) : oidc_device_getVerificationUri(c));
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "QRencode cmd: %s", cmd);
    system(cmd);
    clearFreeString(cmd);
    // printQrCode(oidc_device_getVerificationUriComplete(c) ?: oidc_device_getVerificationUri(c));
  }
}
