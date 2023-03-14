#ifndef NO_STATLOG
#ifndef OIDC_AGENT_STATID_H
#define OIDC_AGENT_STATID_H

struct statid {
  char* machine_id;
  char* boot_id;
  char* os_info;
  char* location;
};

struct statid getStatID();

#endif  // OIDC_AGENT_STATID_H
#endif  // NO_STATLOG