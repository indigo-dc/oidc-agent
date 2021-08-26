#include "deviceCodeEntry.h"

#include "utils/matcher.h"
#include "utils/memory.h"
#include "utils/string/stringUtils.h"

void secFreeDeviceCodeEntryContent(struct deviceCodeEntry* entry) {
  secFreeAccount(entry->account);
  secFree(entry->device_code);
}

struct deviceCodeEntry* createDeviceCodeEntry(const char*          device_code,
                                              struct oidc_account* account) {
  struct deviceCodeEntry* entry = secAlloc(sizeof(struct deviceCodeEntry));
  entry->account                = account;
  entry->device_code            = oidc_strcopy(device_code);
  return entry;
}

int dce_match(struct deviceCodeEntry* a, struct deviceCodeEntry* b) {
  return matchStrings(a->device_code, b->device_code);
}
