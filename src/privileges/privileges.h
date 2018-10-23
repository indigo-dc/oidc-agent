#ifndef OIDCAGENT_PRIVILEGES_H
#define OIDCAGENT_PRIVILEGES_H

#include <seccomp.h>

#ifndef ALLOW_SYSCALL
#define ALLOW_SYSCALL(ctx, call)                                         \
  do {                                                                   \
    int rc = seccomp_rule_add((ctx), SCMP_ACT_ALLOW, SCMP_SYS(call), 0); \
    checkRc(rc, "seccomp_rule_add");                                     \
  } while (0)
#endif  // ALLOW_SYSCALL
#ifndef ALLOW_SYSCALL_PARAM
#define ALLOW_SYSCALL_PARAM(ctx, call, param)                                \
  do {                                                                       \
    int rc =                                                                 \
        seccomp_rule_add((ctx), SCMP_ACT_ALLOW, SCMP_SYS(call), 1, (param)); \
    checkRc(rc, "seccomp_rule_add");                                         \
  } while (0)
#endif  // ALLOW_SYSCALL_PARAM

void checkRc(int rc, const char* str);
void addSocketSysCalls(scmp_filter_ctx ctx);
void addLoggingSysCalls(scmp_filter_ctx ctx);
void addPromptingSysCalls(scmp_filter_ctx ctx);
void addMemorySysCalls(scmp_filter_ctx ctx);
void addGeneralSysCalls(scmp_filter_ctx ctx);
void addTimeSysCalls(scmp_filter_ctx ctx);
void addFileWriteSysCalls(scmp_filter_ctx ctx);
void addFileReadSysCalls(scmp_filter_ctx ctx);

#endif  // OIDCAGENT_PRIVILEGES_H
