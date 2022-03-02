/**
 * Adapted from
 * qrencode - QR Code encoder
 *
 * QR Code encoding tool
 * Copyright (C) 2006-2017 Kentaro Fukuchi <kentaro@fukuchi.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "qr.h"

#include <errno.h>
#include <qrencode.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "utils/guiChecker.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/string/stringUtils.h"
#include "utils/string/stringbuilder.h"

static const size_t margin = 2;

static void getUTF8_margin(str_builder_t* str, size_t realwidth,
                           const char* white, const char* reset,
                           const char* full) {
  for (size_t y = 0; y < margin / 2; y++) {
    str_builder_add_str(str, white);
    for (size_t x = 0; x < realwidth; x++) { str_builder_add_str(str, full); }
    str_builder_add_str(str, reset);
    str_builder_add_char(str, '\n');
  }
}

static void getANSI_margin(str_builder_t* str, size_t realwidth,
                           const char* white) {
  str_builder_t* row = str_builder_create(
      3 * realwidth);  // we are writing 2*realwidth + color coding; so 3 should
                       // be enough space so no reallocation is needed
  str_builder_add_str(row, white);
  char* spaces = repeatChar(' ', realwidth * 2);
  str_builder_add_str(row, spaces);
  secFree(spaces);
  str_builder_add_str(row, "\033[0m\n");  // reset to default colors
  char* r = str_builder_get_string(row);
  secFree_str_builder(row);
  for (size_t y = 0; y < margin; y++) { str_builder_add_str(str, r); }
  secFree(r);
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

  size_t realwidth = (qrcode->width + margin * 2);

  str_builder_t* str = str_builder_create(1024);

  /* top margin */
  getUTF8_margin(str, realwidth, white, reset, full);

  /* data */
  for (int y = 0; y < qrcode->width; y += 2) {
    unsigned char* row1 = qrcode->data + y * qrcode->width;
    unsigned char* row2 = row1 + qrcode->width;

    str_builder_add_str(str, white);

    for (size_t x = 0; x < margin; x++) { str_builder_add_str(str, full); }

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

    for (size_t x = 0; x < margin; x++) { str_builder_add_str(str, full); }

    str_builder_add_str(str, reset);
    str_builder_add_char(str, '\n');
  }

  /* bottom margin */
  getUTF8_margin(str, realwidth, white, reset, full);

  char* qr = str_builder_get_string(str);
  secFree_str_builder(str);
  return qr;
}

static char* getANSI(const QRcode* qrcode, int ansi256) {
  const char* white = ansi256 ? "\033[48;5;231m" : "\033[47m";
  const char* black = ansi256 ? "\033[48;5;16m" : "\033[40m";

  size_t realwidth = (qrcode->width + margin * 2);

  str_builder_t* str = str_builder_create(1024);

  /* top margin */
  getANSI_margin(str, realwidth, white);

  /* data */
  unsigned char* p = qrcode->data;
  char*          m = repeatChar(' ', 2 * margin);
  for (int y = 0; y < qrcode->width; y++) {
    unsigned char* row = (p + (y * qrcode->width));

    str_builder_add_str(str, white);
    str_builder_add_str(str, m);

    unsigned char last = 0;

    for (int x = 0; x < qrcode->width; x++) {
      if (*(row + x) & 0x1) {
        if (last != 1) {
          str_builder_add_str(str, black);
          last = 1;
        }
      } else if (last != 0) {
        str_builder_add_str(str, white);
        last = 0;
      }
      str_builder_add_str(str, "  ");
    }

    if (last != 0) {
      str_builder_add_str(str, white);
    }
    str_builder_add_str(str, m);
    str_builder_add_str(str, "\033[0m\n");
  }
  secFree(m);

  /* bottom margin */
  getANSI_margin(str, realwidth, white);

  char* qr = str_builder_get_string(str);
  secFree_str_builder(str);
  return qr;
}

static void writeSVG_drawModules(FILE* fp, int x, int y, int width,
                                 const char* col) {
  fprintf(fp,
          "\t\t\t<rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"1\" "
          "fill=\"#%s\"/>\n",
          x, y, width, col);
}

static int writeSVG(const QRcode* qrcode, FILE* fp, size_t size) {
  const size_t dpi       = 72;
  const float  scale     = dpi / 2.54;
  size_t       symwidth  = qrcode->width + margin * 2;
  size_t       realwidth = symwidth * size;

  const char fg[7] = {'0', '0', '0', '0', '0', '0', 0};
  const char bg[7] = {'f', 'f', 'f', 'f', 'f', 'f', 0};

  /* XML declaration */
  fputs("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n", fp);

  /* DTD
     No document type specified because "while a DTD is provided in [the SVG]
     specification, the use of DTDs for validating XML documents is known to
     be problematic. In particular, DTDs do not handle namespaces gracefully.
     It is *not* recommended that a DOCTYPE declaration be included in SVG
     documents."
     http://www.w3.org/TR/2003/REC-SVG11-20030114/intro.html#Namespace
  */

  /* Vanity remark */
  fprintf(fp,
          "<!-- Created with oidc-prompt and libqrencode %s "
          "(https://fukuchi.org/works/qrencode/index.html) -->\n",
          QRcode_APIVersionString());

  /* SVG code start */
  fprintf(fp,
          "<svg width=\"%.2fcm\" height=\"%.2fcm\" viewBox=\"0 0 %lu %lu\""
          " preserveAspectRatio=\"none\" version=\"1.1\""
          " xmlns=\"http://www.w3.org/2000/svg\">\n",
          realwidth / scale, realwidth / scale, symwidth, symwidth);

  /* Make named group */
  fputs("\t<g id=\"QRcode\">\n", fp);

  /* Make background */
  fprintf(
      fp,
      "\t\t<rect x=\"0\" y=\"0\" width=\"%lu\" height=\"%lu\" fill=\"#%s\"/>\n",
      symwidth, symwidth, bg);

  /* Create new viewbox for QR data */
  fprintf(fp, "\t\t<g id=\"Pattern\" transform=\"translate(%lu,%lu)\">\n",
          margin, margin);

  /* Write data */
  unsigned char* p = qrcode->data;
  for (int y = 0; y < qrcode->width; y++) {
    unsigned char* row = (p + (y * qrcode->width));

    /* no RLE */
    for (int x = 0; x < qrcode->width; x++) {
      if (*(row + x) & 0x1) {
        writeSVG_drawModules(fp, x, y, 1, fg);
      }
    }
  }

  /* Close QR data viewbox */
  fputs("\t\t</g>\n", fp);

  /* Close group */
  fputs("\t</g>\n", fp);

  /* Close SVG code */
  fputs("</svg>\n", fp);

  return 0;
}

static int writeXPM(const QRcode* qrcode, FILE* fp, size_t size) {
  size_t realwidth  = (qrcode->width + margin * 2) * size;
  size_t realmargin = margin * size;

  char* row = secAlloc((size_t)realwidth + 1);
  if (row == NULL) {
    return oidc_errno;
  }
  const char fg[7] = {'0', '0', '0', '0', '0', '0', 0};
  const char bg[7] = {'f', 'f', 'f', 'f', 'f', 'f', 0};

  fputs("/* XPM */\n", fp);
  fputs("static const char *const qrcode_xpm[] = {\n", fp);
  fputs("/* width height ncolors chars_per_pixel */\n", fp);
  fprintf(fp, "\"%lu %lu 2 1\",\n", realwidth, realwidth);

  fputs("/* colors */\n", fp);
  fprintf(fp, "\"F c #%s\",\n", fg);
  fprintf(fp, "\"B c #%s\",\n", bg);

  fputs("/* pixels */\n", fp);
  memset(row, 'B', (size_t)realwidth);
  row[realwidth] = '\0';

  for (size_t y = 0; y < realmargin; y++) { fprintf(fp, "\"%s\",\n", row); }

  unsigned char* p = qrcode->data;
  for (int y = 0; y < qrcode->width; y++) {
    for (size_t yy = 0; yy < size; yy++) {
      fputs("\"", fp);

      for (size_t x = 0; x < margin; x++) {
        for (size_t xx = 0; xx < size; xx++) { fputs("B", fp); }
      }

      for (int x = 0; x < qrcode->width; x++) {
        for (size_t xx = 0; xx < size; xx++) {
          if (p[(y * qrcode->width) + x] & 0x1) {
            fputs("F", fp);
          } else {
            fputs("B", fp);
          }
        }
      }

      for (size_t x = 0; x < margin; x++) {
        for (size_t xx = 0; xx < size; xx++) { fputs("B", fp); }
      }

      fputs("\",\n", fp);
    }
  }

  for (size_t y = 0; y < realmargin; y++) {
    fprintf(fp, "\"%s\"%s\n", row, y < (size - 1) ? "," : "};");
  }

  secFree(row);
  return 0;
}

static int writeEPS(const QRcode* qrcode, FILE* fp, size_t size) {
  const float fg[3] = {(float)0., (float)0., (float)0.};
  const float bg[3] = {(float)1., (float)1., (float)1.};

  size_t realwidth = (qrcode->width + margin * 2) * size;
  /* EPS file header */
  fprintf(fp,
          "%%!PS-Adobe-2.0 EPSF-1.2\n"
          "%%%%BoundingBox: 0 0 %lu %lu\n"
          "%%%%Pages: 1 1\n"
          "%%%%EndComments\n",
          realwidth, realwidth);
  /* draw point */
  fprintf(fp, "/p { "
              "moveto "
              "0 1 rlineto "
              "1 0 rlineto "
              "0 -1 rlineto "
              "fill "
              "} bind def\n");
  /* set color */
  fprintf(fp, "gsave\n");
  fprintf(fp, "%f %f %f setrgbcolor\n", bg[0], bg[1], bg[2]);
  fprintf(fp, "%lu %lu scale\n", realwidth, realwidth);
  fprintf(fp, "0 0 p\ngrestore\n");
  fprintf(fp, "%f %f %f setrgbcolor\n", fg[0], fg[1], fg[2]);
  fprintf(fp, "%lu %lu scale\n", size, size);

  /* data */
  unsigned char* p = qrcode->data;
  for (int y = 0; y < qrcode->width; y++) {
    unsigned char* row = (p + (y * qrcode->width));
    int            yy  = (int)(margin + qrcode->width - y - 1);

    for (int x = 0; x < qrcode->width; x++) {
      if (*(row + x) & 0x1) {
        fprintf(fp, "%lu %d p ", margin + x, yy);
      }
    }
  }

  fprintf(fp, "\n%%%%EOF\n");
  return 0;
}

#define IMG_FILE_XPM 1
#define IMG_FILE_ESP 2
#define IMG_FILE_SVG 3

#define IMG_DEFAULT_FILE_TYPE IMG_FILE_SVG

static int writeIMGFile(const QRcode* qrcode, const char* outfile,
                        unsigned char format) {
  FILE* fp = fopen(outfile, "wb");
  if (fp == NULL) {
    return errno;
  }
  int (*imageWriter)(const QRcode*, FILE*, size_t);
  switch (format) {
    case IMG_FILE_SVG: imageWriter = writeSVG; break;
    case IMG_FILE_XPM: imageWriter = writeXPM; break;
    case IMG_FILE_ESP: imageWriter = writeEPS; break;
    default:
      oidc_setInternalError("unknown QR image file format");
      return oidc_errno;
  }
  int ret = imageWriter(qrcode, fp, 3);
  fclose(fp);
  return ret;
}

char* getQRCode(const char* content) {
  return GUIAvailable() ? getUTF8QRCode(content) : getANSIQRCode(content);
}

char* getUTF8QRCode(const char* content) {
  QRcode* qr = QRcode_encodeString(content, 0, QR_ECLEVEL_L, QR_MODE_8, 1);
  if (qr == NULL) {
    return NULL;
  }
  char* ret = getUTF8(qr, 1, 0);
  QRcode_free(qr);
  return ret;
}

char* getANSIQRCode(const char* content) {
  QRcode* qr = QRcode_encodeString(content, 0, QR_ECLEVEL_L, QR_MODE_8, 1);
  if (qr == NULL) {
    return NULL;
  }
  char* ret = getANSI(qr, 1);
  QRcode_free(qr);
  return ret;
}

int getIMGQRCode(const char* content, const char* outpath) {
  QRcode* qr = QRcode_encodeString(content, 0, QR_ECLEVEL_L, QR_MODE_8, 1);
  if (qr == NULL) {
    return -1;
  }
  int ret = writeIMGFile(qr, outpath, IMG_DEFAULT_FILE_TYPE);
  QRcode_free(qr);
  return ret;
}
