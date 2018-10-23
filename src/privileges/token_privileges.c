// #define _XOPEN_SOURCE 500
#include "token_privileges.h"
#include "privileges.h"

#include <seccomp.h>
#include <stdio.h>
#include <stdlib.h>

// #include <unistd.h>

void initOidcTokenPrivileges(struct arguments* arguments) {
  int             rc  = -1;
  scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_KILL);
  if (ctx == NULL) {
    perror("seccomp_init");
    exit(EXIT_FAILURE);
  }
  rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(lseek), 0);
  addGeneralSysCalls(ctx);
  addLoggingSysCalls(ctx);
  addSocketSysCalls(ctx);

  rc = seccomp_load(ctx);
  seccomp_release(ctx);
  checkRc(rc, "seccomp_load");
  // access("STARTOFPROGRAM", F_OK);
}
