#include "qr.h"

#include <qrencode.h>
#include <stddef.h>

#include "utils/string/stringbuilder.h"

static int margin = 2;

static void getUTF8_margin(str_builder_t* str, int realwidth, const char* white,
                           const char* reset, const char* full) {
  for (int y = 0; y < margin / 2; y++) {
    str_builder_add_str(str, white);
    for (int x = 0; x < realwidth; x++) { str_builder_add_str(str, full); }
    str_builder_add_str(str, reset);
    str_builder_add_char(str, '\n');
  }
}

static char* getUTF8(const QRcode* qrcode, int use_ansi, int invert) {
  const char *white, *reset;

  const char* empty   = " ";
  const char* lowhalf = "\342\226\204";
  const char* uphalf  = "\342\226\200";
  const char* full    = "\342\226\210";

  if (invert) {
    const char* tmp = empty;
    empty           = full;
    full            = tmp;

    tmp     = lowhalf;
    lowhalf = uphalf;
    uphalf  = tmp;
  }

  if (use_ansi) {
    if (use_ansi == 2) {
      white = "\033[38;5;231m\033[48;5;16m";
    } else {
      white = "\033[40;37;1m";
    }
    reset = "\033[0m";
  } else {
    white = "";
    reset = "";
  }

  int realwidth = (qrcode->width + margin * 2);

  str_builder_t* str = str_builder_create(1024);

  /* top margin */
  getUTF8_margin(str, realwidth, white, reset, full);

  /* data */
  for (int y = 0; y < qrcode->width; y += 2) {
    unsigned char* row1 = qrcode->data + y * qrcode->width;
    unsigned char* row2 = row1 + qrcode->width;

    str_builder_add_str(str, white);

    for (int x = 0; x < margin; x++) { str_builder_add_str(str, full); }

    for (int x = 0; x < qrcode->width; x++) {
      if (row1[x] & 1) {
        if (y < qrcode->width - 1 && row2[x] & 1) {
          str_builder_add_str(str, empty);
        } else {
          str_builder_add_str(str, lowhalf);
        }
      } else if (y < qrcode->width - 1 && row2[x] & 1) {
        str_builder_add_str(str, uphalf);
      } else {
        str_builder_add_str(str, full);
      }
    }

    for (int x = 0; x < margin; x++) { str_builder_add_str(str, full); }

    str_builder_add_str(str, reset);
    str_builder_add_char(str, '\n');
  }

  /* bottom margin */
  getUTF8_margin(str, realwidth, white, reset, full);

  char* qr = str_builder_get_string(str);
  secFree_str_builder(str);
  return qr;
}

char* getQRCode(const char* content) {
  QRcode* qr = QRcode_encodeString(content, 0, QR_ECLEVEL_L, QR_MODE_8, 1);
  if (qr == NULL) {
    return NULL;
  }
  return getUTF8(qr, 1, 0);
}
