#include "privileges.h"

#include <seccomp.h>
#include <stdio.h>
#include <stdlib.h>

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
  ALLOW_SYSCALL(ctx, write);
}

void addFileReadSysCalls(scmp_filter_ctx ctx) {
  ALLOW_SYSCALL(ctx, access);
  ALLOW_SYSCALL(ctx, open);  // TODO This can be restricted
  ALLOW_SYSCALL(ctx, read);
  ALLOW_SYSCALL(ctx, fstat);
  ALLOW_SYSCALL(ctx, getdents);
}

void addFileWriteSysCalls(scmp_filter_ctx ctx) {
  addFileReadSysCalls(ctx);
  ALLOW_SYSCALL(ctx, write);
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
  addPrintingSysCalls(ctx);
  ALLOW_SYSCALL(ctx, read);   // TODO restrict to stdin
  ALLOW_SYSCALL(ctx, ioctl);  // TODO restrict to stdin
}

void addPrintingSysCalls(scmp_filter_ctx ctx) {
  ALLOW_SYSCALL(ctx, write);  // TODO restrict to stderr stdout
  ALLOW_SYSCALL(ctx, fstat);  // TODO restrict to stdin stderr stdout
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
  addPrintingSysCalls(ctx);
}
