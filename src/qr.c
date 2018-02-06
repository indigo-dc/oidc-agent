#include "qr.h"

#include "../lib/qr/c/qrcodegen.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

static void printQr(const uint8_t qrcode[]) {
  int size = qrcodegen_getSize(qrcode);
  int border = 4;
  int y, x;
  for (y = -border; y < size + border; y++) {
    for (x = -border; x < size + border; x++) {
      fputs((qrcodegen_getModule(qrcode, x, y) ? "##" : "  "), stdout);
    }
    fputs("\n", stdout);
  }
  fputs("\n", stdout);
}

int printQrCode(const char* url) {
  enum qrcodegen_Ecc errCorLvl = qrcodegen_Ecc_LOW;  // Error correction level

  // Make and print the QR Code symbol
  uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
  uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
  bool ok = qrcodegen_encodeText(url, tempBuffer, qrcode, errCorLvl,
      qrcodegen_VERSION_MIN, 40, qrcodegen_Mask_AUTO, true);
  if (ok) {
    printQr(qrcode);
    return 1;
  } else {
    return 0;
  }
}
