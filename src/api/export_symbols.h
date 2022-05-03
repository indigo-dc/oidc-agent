#ifndef OIDC_EXPORT_SYMBOLS_H
#define OIDC_EXPORT_SYMBOLS_H

#if defined __MINGW32__ || defined __MINGW64__
#define LIB_PUBLIC __declspec(dllexport)
#else

//#if defined _WIN32 || defined __CYGWIN__
#if defined _WIN32
#ifdef BUILDING_LIB
#ifdef __GNUC__
#define LIB_PUBLIC __attribute__((dllexport))
#else
#define LIB_PUBLIC \
  __declspec(      \
      dllexport)  // Note: actually gcc seems to also supports this syntax.
#endif
#else
#ifdef __GNUC__
#define LIB_PUBLIC __attribute__((dllimport))
#else
#define LIB_PUBLIC \
  __declspec(      \
      dllimport)  // Note: actually gcc seems to also supports this syntax.
#endif
#endif
#define LIB_LOCAL
#else
#if __GNUC__ >= 4
#define LIB_PUBLIC __attribute__((visibility("default")))
#define LIB_LOCAL __attribute__((visibility("hidden")))
#else
#define LIB_PUBLIC
#define LIB_LOCAL
#endif
#endif

#endif

#endif  // OIDC_EXPORT_SYMBOLS_H
