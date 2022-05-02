#ifndef OIDC_AGENT_MSYS_H
#define OIDC_AGENT_MSYS_H

#if defined __MINGW32__ || defined __MINGW64__
#define MINGW
#endif

#if defined MINGW || defined __MSYS__
#define ANY_MSYS
#endif

#ifndef ANY_MSYS
#define NO_MSYS
#endif

#endif  // OIDC_AGENT_MSYS_H
