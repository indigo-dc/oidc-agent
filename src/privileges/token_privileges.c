#include "token_privileges.h"

#include <seccomp.h>
#include <stdio.h>
#include <stdlib.h>

#include "privileges.h"

void initOidcTokenPrivileges(
    __attribute__((unused)) struct arguments* arguments) {
  int             rc  = -1;
  scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_KILL);
  if (ctx == NULL) {
    perror("seccomp_init");
    exit(EXIT_FAILURE);
  }
  addGeneralSysCalls(ctx);
  addLoggingSysCalls(ctx);
  addSocketSysCalls(ctx);

  rc = seccomp_load(ctx);
  seccomp_release(ctx);
  checkRc(rc, "seccomp_load", "");
  // access("STARTOFPROGRAM", F_OK);
}
