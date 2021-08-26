#include "gpg.h"

#include <sodium.h>
#include <string.h>

#include "defines/version.h"
#include "utils/crypt/hexCrypt.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/logger.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/string/stringUtils.h"
#include "utils/system_runner.h"
#include "utils/versionUtils.h"

char* decryptPGPMessage(const char* gpg) {
  char* cmd   = oidc_sprintf("echo '%s' | gpg -d 2>/dev/null", gpg);
  char* plain = getOutputFromCommand(cmd);
  secFree(cmd);
  return plain;
}

char* decryptPGPFileContent(const char* content) {
  char* copy  = oidc_strcopy(content);
  char* index = strstr(copy, "END PGP MESSAGE");
  index       = strchr(index, '\n');
  *index      = '\0';  // ignore everything after the end of the pgp message
  char* plain = decryptPGPMessage(copy);
  secFree(copy);
  if (plain == NULL) {
    oidc_errno = OIDC_EDECRYPT;
  }
  return plain;
}

unsigned char isPGPMessage(const char* content) {
  return strstarts(content, "-----BEGIN PGP MESSAGE-----");
}

unsigned char isPGPOIDCFile(const char* shortname) {
  char* encrypted = readOidcFile(shortname);
  if (encrypted == NULL) {
    return 0;
  }
  unsigned char ret = isPGPMessage(encrypted);
  secFree(encrypted);
  return ret;
}

/**
 * @brief encrypts a given text with the given gpg key id and adds the current
 * oidc-agent version
 * @return the encrypted text in a formatted string that holds all relevant
 * encryption information as well as the oidc-agent version. Can be passed to
 * @c decryptPGPFileContent
 */
char* encryptPGPWithVersionLine(const char* text, const char* gpg_key) {
  char* cmd   = oidc_sprintf("echo '%s' | gpg -e -r %s --armor", text, gpg_key);
  char* crypt = getOutputFromCommand(cmd);
  secFree(cmd);
  if (crypt == NULL) {
    return NULL;
  }
  char* version_line = simpleVersionToVersionLine(VERSION);
  char* ret          = oidc_sprintf("%s\n%s", crypt, version_line);
  secFree(crypt);
  secFree(version_line);
  return ret;
}

#define returnIfNULL(X) \
  if (X == NULL) {      \
    return NULL;        \
  }

char* extractPGPKeyIDFromOIDCFile(const char* shortname) {
  char* encrypted = readOidcFile(shortname);
  if (encrypted == NULL) {
    return NULL;
  }
  if (!isPGPMessage(encrypted)) {
    secFree(encrypted);
    return NULL;
  }
  char* key = extractPGPKeyID(encrypted);
  secFree(encrypted);
  return key;
}

char* extractPGPKeyID(const char* text) {
  char* copy  = oidc_strcopy(text);
  char* index = strstr(copy, "BEGIN PGP MESSAGE");
  returnIfNULL(index);
  index = strstr(index, "\n\n");
  returnIfNULL(index);
  char* begin = index + 2;
  index       = strstr(copy, "END PGP MESSAGE");
  returnIfNULL(index);
  *index = '\0';
  index  = strrchr(begin, '\n');
  returnIfNULL(index);
  *index             = '\0';
  unsigned char* bin = secAlloc(2 * strlen(text));
  // At this point begin points to string containing everything within the PGP
  // MESSAGE blog We still have to remove the last line, since this is not in
  // the same base64 block
  index = strrchr(begin, '\n');
  returnIfNULL(index);
  *index = '\0';
  //  strelim(begin, '\n');

  if (sodium_base642bin(bin, strlen(text), begin, strlen(begin), " \t\n", NULL,
                        NULL, sodium_base64_VARIANT_ORIGINAL)) {
    secFree(bin);
    secFree(copy);
    return NULL;
  }
  char* keyID = toHex(
      bin + 4, 8);  // keyid is 8byte long starting at the 4th byte
                    // https://datatracker.ietf.org/doc/html/rfc4880#section-5.1
  secFree(bin);
  secFree(copy);
  return keyID;
}
