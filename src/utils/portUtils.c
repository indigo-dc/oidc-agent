#define _XOPEN_SOURCE 500
#include "portUtils.h"

#include "account/account.h"
#include "list/list.h"
#include "oidc_error.h"
#include "stringUtils.h"

#include <stdio.h>
#include <stdlib.h>

long random_at_most(long max) {
  // max <= RAND_MAX < ULONG_MAX, so this is okay.
  unsigned long num_bins = (unsigned long)max + 1,
                num_rand = (unsigned long)RAND_MAX + 1,
                bin_size = num_rand / num_bins, defect = num_rand % num_bins;
  long x;
  do { x = random(); } while (num_rand - defect <= (unsigned long)x);

  return x / bin_size;
}

unsigned short getRandomPort() {
  return random_at_most(MAX_PORT - MIN_PORT) + MIN_PORT;
}

char* portToUri(unsigned short port) {
  return oidc_sprintf("http://localhost:%hu", port);
}

/**
 * don't free the returned value
 */
char* findRedirectUriByPort(const struct oidc_account* a, unsigned short port) {
  list_t*          l  = account_getRedirectUris(a);
  list_iterator_t* it = list_iterator_new(l, LIST_HEAD);
  list_node_t*     node;
  while ((node = list_iterator_next(it))) {
    if (getPortFromUri(node->val) == port) {
      list_iterator_destroy(it);
      return node->val;
    }
  }
  list_iterator_destroy(it);
  return NULL;
}

unsigned int getPortFromUri(const char* uri) {
  unsigned int port;
  if (sscanf(uri, "http://localhost:%du", &port) != 1) {
    if (sscanf(uri, "http://localhost:%du/", &port) != 1) {
      oidc_errno = OIDC_EFMT;
      return 0;
    }
  }
  return port;
}
