#ifndef OIDC_AGENT_DEVICECODEENTRY_H
#define OIDC_AGENT_DEVICECODEENTRY_H

#include "account/account.h"

struct deviceCodeEntry {
  char*                device_code;
  struct oidc_account* account;
};

int dce_match(struct deviceCodeEntry* a, struct deviceCodeEntry* b);
struct deviceCodeEntry* createDeviceCodeEntry(const char*          device_code,
                                              struct oidc_account* account);
void                    secFreeDeviceCodeEntryContent(struct deviceCodeEntry*);

#endif  // OIDC_AGENT_DEVICECODEENTRY_H
