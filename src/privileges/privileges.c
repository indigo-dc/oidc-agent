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
  ALLOW_SYSCALL(ctx, fstat);
  ALLOW_SYSCALL(ctx, lseek);
}

void addFileReadSysCalls(scmp_filter_ctx ctx) {
  ALLOW_SYSCALL(ctx, access);
  ALLOW_SYSCALL(ctx, open);  // TODO This can be restricted
  ALLOW_SYSCALL(ctx, read);
  ALLOW_SYSCALL(ctx, fstat);
  ALLOW_SYSCALL(ctx, getdents);
  ALLOW_SYSCALL(ctx, lseek);
}

void addCryptSysCalls(scmp_filter_ctx ctx) {
  ALLOW_SYSCALL(ctx, getrandom);
  ALLOW_SYSCALL(ctx, open);  // TODO restrict to /dev/urandom
}

void addAgentIpcSysCalls(scmp_filter_ctx ctx) {
  ALLOW_SYSCALL(ctx, mkdir);   // TODO restrict to tmp
  ALLOW_SYSCALL(ctx, unlink);  // TODO restrict to tmp
  ALLOW_SYSCALL(ctx, bind);
  ALLOW_SYSCALL(ctx, fcntl);
  ALLOW_SYSCALL(ctx, listen);
  ALLOW_SYSCALL(ctx, accept);
}

void addDaemonSysCalls(scmp_filter_ctx ctx) {
  ALLOW_SYSCALL(ctx, clone);
  ALLOW_SYSCALL(ctx, getppid);
  ALLOW_SYSCALL(ctx, rt_sigaction);
  ALLOW_SYSCALL(ctx, set_robust_list);
  ALLOW_SYSCALL(ctx, setsid);
  ALLOW_SYSCALL(ctx, chdir);  // TODO restrict to "/"
  ALLOW_SYSCALL(ctx, umask);
  ALLOW_SYSCALL(ctx, open);  // TODO restrict to /dev/null
}

void addKillSysCall(scmp_filter_ctx ctx) { ALLOW_SYSCALL(ctx, kill); }

void addHttpSysCalls(scmp_filter_ctx ctx) {
  ALLOW_SYSCALL(ctx, pipe2);
  ALLOW_SYSCALL(ctx, poll);
  ALLOW_SYSCALL(ctx, futex);
  ALLOW_SYSCALL(ctx, access);
  ALLOW_SYSCALL(ctx, stat);
  ALLOW_SYSCALL(ctx, sendmmsg);
  ALLOW_SYSCALL(ctx, recvfrom);
  ALLOW_SYSCALL(ctx, setsockopt);
  ALLOW_SYSCALL(ctx, getsockopt);
  ALLOW_SYSCALL(ctx, getpeername);
  ALLOW_SYSCALL(ctx, getsockname);
  ALLOW_SYSCALL(ctx, sysinfo);
  ALLOW_SYSCALL(ctx, getuid);
  ALLOW_SYSCALL(ctx, madvise);
}

void addHttpServerSysCalls(scmp_filter_ctx ctx) {
  ALLOW_SYSCALL(ctx,
                prctl);  // TODO restrict to prctl(PR_SET_PDEATHSIG, SIGTERM)
  ALLOW_SYSCALL(ctx, kill);
}

void addFileWriteSysCalls(scmp_filter_ctx ctx) {
  addFileReadSysCalls(ctx);
  ALLOW_SYSCALL(ctx, write);
}

void addTimeSysCalls(scmp_filter_ctx ctx) {
  // ALLOW_SYSCALL_PARAM(ctx, open,
  //                     SCMP_A0(SCMP_CMP_EQ, (scmp_datum_t)
  //                     "/etc/localtime"));
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
  ALLOW_SYSCALL(ctx, mprotect);
}

void addGeneralSysCalls(scmp_filter_ctx ctx) {
  ALLOW_SYSCALL(ctx, exit);
  ALLOW_SYSCALL(ctx, exit_group);
  ALLOW_SYSCALL(ctx, brk);
  addMemorySysCalls(ctx);
  addPrintingSysCalls(ctx);
}

void addSignalHandlingSysCalls(scmp_filter_ctx ctx) {
  ALLOW_SYSCALL(ctx, rt_sigaction);
  ALLOW_SYSCALL(ctx, rt_sigprocmask);
}

void addSleepSysCalls(scmp_filter_ctx ctx) { ALLOW_SYSCALL(ctx, nanosleep); }

void addExecSysCalls(scmp_filter_ctx ctx) {
  ALLOW_SYSCALL(ctx, clone);
  ALLOW_SYSCALL(ctx, wait4);
  ALLOW_SYSCALL(ctx, execve);
  // would be nice if possible to remove restriction for executed process
  ALLOW_SYSCALL(ctx, arch_prctl);
  ALLOW_SYSCALL(ctx, geteuid);
  ALLOW_SYSCALL(ctx, getppid);
  ALLOW_SYSCALL(ctx, open);
  ALLOW_SYSCALL(ctx, read);
  ALLOW_SYSCALL(ctx, stat);
  ALLOW_SYSCALL(ctx, fstat);
  ALLOW_SYSCALL(ctx, fcntl);
  ALLOW_SYSCALL(ctx, rt_sigaction);
  ALLOW_SYSCALL(ctx, rt_sigprocmask);
  ALLOW_SYSCALL(ctx, rt_sigreturn);
  ALLOW_SYSCALL(ctx, set_tid_address);
  ALLOW_SYSCALL(ctx, set_robust_list);
  ALLOW_SYSCALL(ctx, getrlimit);
  ALLOW_SYSCALL(ctx, statfs);
  ALLOW_SYSCALL(ctx, futex);
  ALLOW_SYSCALL(ctx, access);
}
