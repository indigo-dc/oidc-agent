#include "deathUtils.h"

#include <time.h>

#include "utils/logger.h"
#include "utils/oidc_error.h"

/**
 * @brief returns the minimum death time in an list
 * @param list a list
 * @return the minimum time of death; might be @c 0
 */
time_t getMinDeathFrom(list_t* list, time_t (*deathGetter)(void*)) {
  if (list == NULL) {
    oidc_setArgNullFuncError(__func__);
    return 0;
  }
  time_t           min = 0;
  list_node_t*     node;
  list_iterator_t* it = list_iterator_new(list, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    void*  elem  = node->val;
    time_t death = deathGetter(elem);
    logger(DEBUG, "this death is %lu", death);
    if (death > 0 && (death < min || min == 0) && death > time(NULL)) {
      logger(DEBUG, "updating min to %lu", death);
      min = death;
    }
  }
  list_iterator_destroy(it);
  logger(DEBUG, "Minimum death in list is %lu", min);
  return min;
}

void* getDeathElementFrom(list_t* list, time_t (*deathGetter)(void*)) {
  if (list == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  list_node_t*     node;
  list_iterator_t* it  = list_iterator_new(list, LIST_HEAD);
  time_t           now = time(NULL);
  while ((node = list_iterator_next(it))) {
    void*  elem  = node->val;
    time_t death = deathGetter(elem);
    if (death > 0 && death <= now) {
      list_iterator_destroy(it);
      logger(DEBUG, "Found element died at %lu (current time %lu)", death, now);
      return elem;
    }
  }
  list_iterator_destroy(it);
  logger(DEBUG, "Found no death element");
  return NULL;
}
