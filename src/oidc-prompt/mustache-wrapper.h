#ifndef OIDC_MUSTACHE_H
#define OIDC_MUSTACHE_H

#include <errno.h>
#include <stdio.h>

#ifndef USE_MUSTACHE_SO
#include "mustache/mustach-cjson.h"
#else
#include <mustach/mustach-cjson.h>
#endif /* USE_MUSTACHE_SO */

#endif /* OIDC_MUSTACHE_H */
