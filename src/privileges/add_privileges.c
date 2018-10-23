#define _XOPEN_SOURCE 500
#include "add_privileges.h"
#include "../oidc_error.h"

#include <seccomp.h>
#include <stdlib.h>

// TODO REMOVE
#include <unistd.h>

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

void checkRc(int rc, const char* str) {
  if (rc < 0) {
    perror(str);
    exit(-rc);
  }
}

void addSocketSysCalls(scmp_filter_ctx ctx) {
  ALLOW_SYSCALL(ctx, sendto);
  ALLOW_SYSCALL(ctx, socket);
  ALLOW_SYSCALL(ctx, read);
  ALLOW_SYSCALL(ctx, ioctl);
  ALLOW_SYSCALL(ctx, connect);
  ALLOW_SYSCALL(ctx, close);
  ALLOW_SYSCALL(ctx, select);
}

void addFileSysCalls(scmp_filter_ctx ctx) {
  ALLOW_SYSCALL(ctx, access);
  ALLOW_SYSCALL(ctx, open);
  ALLOW_SYSCALL(ctx, read);
  ALLOW_SYSCALL(ctx, fstat);
  ALLOW_SYSCALL(ctx, getdents);
}

void addTimeSysCalls(scmp_filter_ctx ctx) {
  // ALLOW_SYSCALL_PARAM(ctx, open,
  //                     SCMP_A0(SCMP_CMP_EQ, (scmp_datum_t) "/etc/localtime"));
  ALLOW_SYSCALL(ctx, open);  // TODO should be restricted to /etc/localtime
}

void addLoggingSysCalls(scmp_filter_ctx ctx) {
  addTimeSysCalls(ctx);
  ALLOW_SYSCALL(ctx, getpid);
  ALLOW_SYSCALL(ctx, sendto);
  ALLOW_SYSCALL(ctx, socket);
  ALLOW_SYSCALL(ctx, connect);
}

void addPromptingSysCalls(scmp_filter_ctx ctx) {
  ALLOW_SYSCALL(ctx, write);   // TODO restrict to stderr stdout
  ALLOW_SYSCALL(ctx, read);    // TODO restrict to stdin
  ALLOW_SYSCALL(ctx, ioctl);   // TODO restrict to stdin
  ALLOW_SYSCALL(ctx, access);  // TODO restrict to stdin stderr stdout
  ALLOW_SYSCALL(ctx, fstat);   // TODO restrict to stdin stderr stdout
}

void addMemorySysCalls(scmp_filter_ctx ctx) {
  ALLOW_SYSCALL(ctx, mmap);
  ALLOW_SYSCALL(ctx, munmap);
  // ALLOW_SYSCALL(ctx, mprotect);
}

void addGeneralSysCalls(scmp_filter_ctx ctx) {
  ALLOW_SYSCALL(ctx, exit);
  ALLOW_SYSCALL(ctx, exit_group);
  ALLOW_SYSCALL(ctx, brk);
  addMemorySysCalls(ctx);
}

void initOidcAddPrivileges(struct arguments* arguments) {
  int             rc  = -1;
  scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_KILL);
  if (ctx == NULL) {
    perror("seccomp_init");
    exit(EXIT_FAILURE);
  }
  rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(lseek), 0);
  addGeneralSysCalls(ctx);
  addPromptingSysCalls(ctx);
  addLoggingSysCalls(ctx);
  addSocketSysCalls(ctx);
  if (!(arguments->lock || arguments->unlock)) {
    addFileSysCalls(ctx);
  }

  rc = seccomp_load(ctx);
  seccomp_release(ctx);
  checkRc(rc, "seccomp_load");
  access("/home/gabriel/STARTOFPROGRAM", F_OK);
}
