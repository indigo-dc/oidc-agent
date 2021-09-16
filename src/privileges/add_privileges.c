#include "add_privileges.h"

#include <seccomp.h>
#include <stdio.h>
#include <stdlib.h>

#include "privileges.h"

void initOidcAddPrivileges(struct arguments* arguments) {
  int             rc  = -1;
  scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_KILL);
  if (ctx == NULL) {
    perror("seccomp_init");
    exit(EXIT_FAILURE);
  }
  addGeneralSysCalls(ctx);
  addPromptingSysCalls(ctx);
  addLoggingSysCalls(ctx);
  addSocketSysCalls(ctx);
  if (!(arguments->lock || arguments->unlock)) {
    addFileReadSysCalls(ctx);
  }

  rc = seccomp_load(ctx);
  seccomp_release(ctx);
  checkRc(rc, "seccomp_load", "");
  // access("STARTOFPROGRAM", F_OK);
}
