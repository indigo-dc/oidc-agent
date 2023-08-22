#include "check_file_path.h"

#include <errno.h>
#include <grp.h>

#include "safe_id_range_list.h"
#include "safe_is_path_trusted.h"
#include "utils/oidc_error.h"

oidc_error_t check_socket_path(const char* path, const char* group) {
  struct safe_id_range_list ulist, glist;

  /* initialize the lists of trusted uid/gid, can basically only fail when
   * out of memory.
   */
  if (safe_init_id_range_list(&ulist)) {
    oidc_errno = OIDC_EMEM;
    return oidc_errno;
  }
  if (safe_add_id_to_list(&ulist, (id_t)getuid())) {
    safe_destroy_id_range_list(&ulist);
    oidc_errno = OIDC_EMEM;
    return oidc_errno;
  }
  if (safe_init_id_range_list(&glist)) {
    safe_destroy_id_range_list(&ulist);
    oidc_errno = OIDC_EMEM;
    return oidc_errno;
  }
  if (safe_add_id_to_list(&glist, (id_t)getgid())) {
    safe_destroy_id_range_list(&ulist);
    safe_destroy_id_range_list(&glist);
    oidc_errno = OIDC_EMEM;
    return oidc_errno;
  }
  if (group) {
    struct group* grp = getgrnam(group);
    if (grp == NULL) {
      if (errno == 0) {
        oidc_errno = OIDC_EGROUPNF;
      } else {
        oidc_setErrnoError();
      }
      safe_destroy_id_range_list(&ulist);
      safe_destroy_id_range_list(&glist);
      return oidc_errno;
    }
    if (safe_add_id_to_list(&glist, (id_t)grp->gr_gid)) {
      safe_destroy_id_range_list(&ulist);
      safe_destroy_id_range_list(&glist);
      oidc_errno = OIDC_EMEM;
      return oidc_errno;
    }
  }

  /* Check whether file is trusted */
  int trust = safe_is_path_trusted_r(path, &ulist, &glist);
  /* Check the level of trust */
  switch (trust) {
    case SAFE_PATH_TRUSTED_CONFIDENTIAL:
      oidc_errno = OIDC_SUCCESS;
      break; /* GOOD */
    case SAFE_PATH_UNTRUSTED:
      /* Perms are wrong */
      oidc_errno = OIDC_EPERM;
      break; /* perm error */
    case SAFE_PATH_TRUSTED:
    case SAFE_PATH_TRUSTED_STICKY_DIR:
      /* TRUSTED-only is fine */
      oidc_errno = OIDC_SUCCESS;
      break;
    case SAFE_PATH_ERROR: /* checking failed */
    default:              /* Unknown state, should not be reached */
      oidc_errno = OIDC_EERROR;
      oidc_seterror("unknown socket path checking error");
      break;
  }

  /* free the range lists */
  safe_destroy_id_range_list(&ulist);
  safe_destroy_id_range_list(&glist);

  return oidc_errno;
}