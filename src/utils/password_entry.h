#ifndef OIDCAGENT_PASSWORD_ENTRY_H
#define OIDCAGENT_PASSWORD_ENTRY_H

#include "cJSON/cJSON.h"
#include "defines/agent_values.h"
#include "defines/ipc_values.h"

#include <time.h>

struct password_entry {
  char*         shortname;
  unsigned char type;
  char*         password;
  time_t        expires_at;
  char*         command;
};

#define PW_TYPE_MEM 0x01
#define PW_TYPE_MNG 0x02
#define PW_TYPE_CMD 0x04
#define PW_TYPE_PRMT 0x08

#define PW_KEY_SHORTNAME IPC_KEY_SHORTNAME
#define PW_KEY_TYPE "type"
#define PW_KEY_PASSWORD IPC_KEY_PASSWORD
#define PW_KEY_EXPIRESAT AGENT_KEY_EXPIRESAT
#define PW_KEY_COMMAND "command"

void                   _secFreePasswordEntry(struct password_entry*);
cJSON*                 passwordEntryToJSON(const struct password_entry*);
char*                  passwordEntryToJSONString(const struct password_entry*);
struct password_entry* JSONStringToPasswordEntry(const char*);

void pwe_setPassword(struct password_entry* pw, char* password);
void pwe_setCommand(struct password_entry* pw, char* command);
void pwe_setShortname(struct password_entry* pw, char* shortname);
void pwe_setType(struct password_entry* pw, unsigned char type);
void pwe_setExpiresAt(struct password_entry* pw, time_t expires_at);
void pwe_setExpiresIn(struct password_entry* pw, time_t expires_in);

static inline time_t pwe_getExpiresAt(struct password_entry* pw) {
  return pw ? pw->expires_at : 0;
}

#ifndef secFreePasswordEntry
#define secFreePasswordEntry(ptr) \
  do {                            \
    _secFreePasswordEntry((ptr)); \
    (ptr) = NULL;                 \
  } while (0)
#endif  // secFreePasswordEntry

#endif  // OIDCAGENT_PASSWORD_ENTRY_H
