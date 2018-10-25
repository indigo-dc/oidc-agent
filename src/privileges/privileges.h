#ifndef OIDCAGENT_PRIVILEGES_H
#define OIDCAGENT_PRIVILEGES_H

#include <seccomp.h>

#ifndef ALLOW_SYSCALL
#define ALLOW_SYSCALL(ctx, call)                                        \
  do {                                                                  \
    int rc = seccomp_rule_add((ctx), SCMP_ACT_ALLOW,                    \
                              seccomp_syscall_resolve_name((call)), 0); \
    checkRc(rc, "seccomp_rule_add", (call));                            \
  } while (0)
#endif  // ALLOW_SYSCALL
#ifndef ALLOW_SYSCALL_PARAM
#define ALLOW_SYSCALL_PARAM(ctx, call, param)                                \
  do {                                                                       \
    int rc =                                                                 \
        seccomp_rule_add((ctx), SCMP_ACT_ALLOW, SCMP_SYS(call), 1, (param)); \
    checkRc(rc, "seccomp_rule_add", (call));                                 \
  } while (0)
#endif  // ALLOW_SYSCALL_PARAM

void checkRc(int rc, const char* str, const char* syscall);
void addSocketSysCalls(scmp_filter_ctx ctx);
void addLoggingSysCalls(scmp_filter_ctx ctx);
void addPromptingSysCalls(scmp_filter_ctx ctx);
void addMemorySysCalls(scmp_filter_ctx ctx);
void addGeneralSysCalls(scmp_filter_ctx ctx);
void addTimeSysCalls(scmp_filter_ctx ctx);
void addFileWriteSysCalls(scmp_filter_ctx ctx);
void addFileReadSysCalls(scmp_filter_ctx ctx);
void addPrintingSysCalls(scmp_filter_ctx ctx);
void addCryptSysCalls(scmp_filter_ctx ctx);
void addDaemonSysCalls(scmp_filter_ctx ctx);
void addAgentIpcSysCalls(scmp_filter_ctx ctx);
void addHttpSysCalls(scmp_filter_ctx ctx);
void addHttpServerSysCalls(scmp_filter_ctx ctx);
void addKillSysCall(scmp_filter_ctx ctx);
void addExecSysCalls(scmp_filter_ctx ctx);
void addSignalHandlingSysCalls(scmp_filter_ctx ctx);
void addSleepSysCalls(scmp_filter_ctx ctx);

#endif  // OIDCAGENT_PRIVILEGES_H
