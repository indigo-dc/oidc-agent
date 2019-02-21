#ifndef OIDCAGENT_MARCOS_H
#define OIDCAGENT_MARCOS_H

#define _GET_NTH_ARG(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                     _14, _15, _16, _17, _18, _19, _20, _21, N, ...)         \
  N

#define COUNT_VARARGS(...)                                                   \
  _GET_NTH_ARG("ignored", ##__VA_ARGS__, 20, 19, 18, 17, 16, 15, 14, 13, 12, \
               11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

// Define some macros to help us create overrides based on the
// arity of a for-each-style macro.
#define _fe_0(_call, ...)
#define _fe_1(_call, x) _call(x)
#define _fe_2(_call, x, ...) _call(x) _fe_1(_call, __VA_ARGS__)
#define _fe_3(_call, x, ...) _call(x) _fe_2(_call, __VA_ARGS__)
#define _fe_4(_call, x, ...) _call(x) _fe_3(_call, __VA_ARGS__)
#define _fe_5(_call, x, ...) _call(x) _fe_4(_call, __VA_ARGS__)
#define _fe_6(_call, x, ...) _call(x) _fe_5(_call, __VA_ARGS__)
#define _fe_7(_call, x, ...) _call(x) _fe_6(_call, __VA_ARGS__)
#define _fe_8(_call, x, ...) _call(x) _fe_7(_call, __VA_ARGS__)
#define _fe_9(_call, x, ...) _call(x) _fe_8(_call, __VA_ARGS__)
#define _fe_10(_call, x, ...) _call(x) _fe_9(_call, __VA_ARGS__)
#define _fe_11(_call, x, ...) _call(x) _fe_10(_call, __VA_ARGS__)
#define _fe_12(_call, x, ...) _call(x) _fe_11(_call, __VA_ARGS__)
#define _fe_13(_call, x, ...) _call(x) _fe_12(_call, __VA_ARGS__)
#define _fe_14(_call, x, ...) _call(x) _fe_13(_call, __VA_ARGS__)
#define _fe_15(_call, x, ...) _call(x) _fe_14(_call, __VA_ARGS__)
#define _fe_16(_call, x, ...) _call(x) _fe_15(_call, __VA_ARGS__)
#define _fe_17(_call, x, ...) _call(x) _fe_16(_call, __VA_ARGS__)
#define _fe_18(_call, x, ...) _call(x) _fe_17(_call, __VA_ARGS__)
#define _fe_19(_call, x, ...) _call(x) _fe_18(_call, __VA_ARGS__)
#define _fe_20(_call, x, ...) _call(x) _fe_19(_call, __VA_ARGS__)

#define _fei_0(_call, ...)
#define _fei_1(_call, n, x) _call(n, x)
#define _fei_2(_call, n, x, ...) _call(n, x) _fei_1(_call, n + 1, __VA_ARGS__)
#define _fei_3(_call, n, x, ...) _call(n, x) _fei_2(_call, n + 1, __VA_ARGS__)
#define _fei_4(_call, n, x, ...) _call(n, x) _fei_3(_call, n + 1, __VA_ARGS__)
#define _fei_5(_call, n, x, ...) _call(n, x) _fei_4(_call, n + 1, __VA_ARGS__)
#define _fei_6(_call, n, x, ...) _call(n, x) _fei_5(_call, n + 1, __VA_ARGS__)
#define _fei_7(_call, n, x, ...) _call(n, x) _fei_6(_call, n + 1, __VA_ARGS__)
#define _fei_8(_call, n, x, ...) _call(n, x) _fei_7(_call, n + 1, __VA_ARGS__)
#define _fei_9(_call, n, x, ...) _call(n, x) _fei_8(_call, n + 1, __VA_ARGS__)
#define _fei_10(_call, n, x, ...) _call(n, x) _fei_9(_call, n + 1, __VA_ARGS__)
#define _fei_11(_call, n, x, ...) _call(n, x) _fei_10(_call, n + 1, __VA_ARGS__)
#define _fei_12(_call, n, x, ...) _call(n, x) _fei_11(_call, n + 1, __VA_ARGS__)
#define _fei_13(_call, n, x, ...) _call(n, x) _fei_12(_call, n + 1, __VA_ARGS__)
#define _fei_14(_call, n, x, ...) _call(n, x) _fei_13(_call, n + 1, __VA_ARGS__)
#define _fei_15(_call, n, x, ...) _call(n, x) _fei_14(_call, n + 1, __VA_ARGS__)
#define _fei_16(_call, n, x, ...) _call(n, x) _fei_15(_call, n + 1, __VA_ARGS__)
#define _fei_17(_call, n, x, ...) _call(n, x) _fei_16(_call, n + 1, __VA_ARGS__)
#define _fei_18(_call, n, x, ...) _call(n, x) _fei_17(_call, n + 1, __VA_ARGS__)
#define _fei_19(_call, n, x, ...) _call(n, x) _fei_18(_call, n + 1, __VA_ARGS__)
#define _fei_20(_call, n, x, ...) _call(n, x) _fei_19(_call, n + 1, __VA_ARGS__)

/**
 * Provide a for-each construct for variadic macros. Supports up
 * to 20 args.
 *
 * Example usage1:
 *     #define FWD_DECLARE_CLASS(cls) class cls;
 *     CALL_MACRO_X_FOR_EACH(FWD_DECLARE_CLASS, Foo, Bar)
 *
 * Example usage 2:
 *     #define START_NS(ns) namespace ns {
 *     #define END_NS(ns) }
 *     #define MY_NAMESPACES System, Net, Http
 *     CALL_MACRO_X_FOR_EACH(START_NS, MY_NAMESPACES)
 *     typedef foo int;
 *     CALL_MACRO_X_FOR_EACH(END_NS, MY_NAMESPACES)
 */
#define CALL_MACRO_X_FOR_EACH(x, ...)                                         \
  _GET_NTH_ARG("ignored", ##__VA_ARGS__, _fe_20, _fe_19, _fe_18, _fe_17,      \
               _fe_16, _fe_15, _fe_14, _fe_13, _fe_12, _fe_11, _fe_10, _fe_9, \
               _fe_8, _fe_7, _fe_6, _fe_5, _fe_4, _fe_3, _fe_2, _fe_1, _fe_0) \
  (x, ##__VA_ARGS__)

#define CALL_MACRO_X_FOR_EACH_WITH_N(x, ...)                                   \
  _GET_NTH_ARG("ignored", ##__VA_ARGS__, _fei_20, _fei_19, _fei_18, _fei_17,   \
               _fei_16, _fei_15, _fei_14, _fei_13, _fei_12, _fei_11, _fei_10,  \
               _fei_9, _fei_8, _fei_7, _fei_6, _fei_5, _fei_4, _fei_3, _fei_2, \
               _fei_1, _fei_0)                                                 \
  (x, 0, ##__VA_ARGS__)
#endif  // OIDCAGENT_MARCOS_H
