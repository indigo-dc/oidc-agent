#ifndef OIDC_AGENT_MEMORY_H
#define OIDC_AGENT_MEMORY_H

extern LIB_PUBLIC void _secFree(void*);

#ifndef secFree
#define secFree(ptr) \
do {               \
_secFree((ptr)); \
(ptr) = NULL;    \
} while (0)
#endif  // secFree

#endif //OIDC_AGENT_MEMORY_H
