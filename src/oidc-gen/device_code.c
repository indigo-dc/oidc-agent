#include "device_code.h"

#include "oidc-agent/oidc/device_code.h"
#include "oidc-gen/qr.h"
#include "utils/logger.h"
#include "utils/printer.h"
#include "utils/string/stringUtils.h"

void printDeviceCode(struct oidc_device_code c) {
  printNormal(
      "\nUsing a browser on another device, visit:\n%s\n\nAnd enter the "
      "code: %s\n",
      oidc_device_getVerificationUri(c), oidc_device_getUserCode(c));
  char* qr = getQRCode(strValid(oidc_device_getVerificationUriComplete(c))
                           ? oidc_device_getVerificationUriComplete(c)
                           : oidc_device_getVerificationUri(c));
  if (qr == NULL) {
    logger(NOTICE, "Could not create QR code");
  } else {
    printNormal("Alternatively you can use the following QR code to visit the "
                "above listed URL.\n\n%s\n",
                qr);
    secFree(qr);
  }
}
