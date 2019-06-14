#ifndef OIDCAGENT_MARCOS_H
#define OIDCAGENT_MARCOS_H

#define _GET_NTH_ARG(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                     _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24,  \
                     _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35,  \
                     _36, _37, _38, _39, _40, _41, N, ...)                   \
  N

#define COUNT_VARARGS(...)                                                     \
  _GET_NTH_ARG("ignored", ##__VA_ARGS__, 40, 39, 38, 37, 36, 35, 34, 33, 32,   \
               31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, \
               15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

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
#define _fe_21(_call, x, ...) _call(x) _fe_20(_call, __VA_ARGS__)
#define _fe_22(_call, x, ...) _call(x) _fe_21(_call, __VA_ARGS__)
#define _fe_23(_call, x, ...) _call(x) _fe_22(_call, __VA_ARGS__)
#define _fe_24(_call, x, ...) _call(x) _fe_23(_call, __VA_ARGS__)
#define _fe_25(_call, x, ...) _call(x) _fe_24(_call, __VA_ARGS__)
#define _fe_26(_call, x, ...) _call(x) _fe_25(_call, __VA_ARGS__)
#define _fe_27(_call, x, ...) _call(x) _fe_26(_call, __VA_ARGS__)
#define _fe_28(_call, x, ...) _call(x) _fe_27(_call, __VA_ARGS__)
#define _fe_29(_call, x, ...) _call(x) _fe_28(_call, __VA_ARGS__)
#define _fe_30(_call, x, ...) _call(x) _fe_29(_call, __VA_ARGS__)
#define _fe_31(_call, x, ...) _call(x) _fe_30(_call, __VA_ARGS__)
#define _fe_32(_call, x, ...) _call(x) _fe_31(_call, __VA_ARGS__)
#define _fe_33(_call, x, ...) _call(x) _fe_32(_call, __VA_ARGS__)
#define _fe_34(_call, x, ...) _call(x) _fe_33(_call, __VA_ARGS__)
#define _fe_35(_call, x, ...) _call(x) _fe_34(_call, __VA_ARGS__)
#define _fe_36(_call, x, ...) _call(x) _fe_35(_call, __VA_ARGS__)
#define _fe_37(_call, x, ...) _call(x) _fe_36(_call, __VA_ARGS__)
#define _fe_38(_call, x, ...) _call(x) _fe_37(_call, __VA_ARGS__)
#define _fe_39(_call, x, ...) _call(x) _fe_38(_call, __VA_ARGS__)
#define _fe_40(_call, x, ...) _call(x) _fe_39(_call, __VA_ARGS__)

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
#define _fei_21(_call, n, x, ...) _call(n, x) _fei_20(_call, n + 1, __VA_ARGS__)
#define _fei_22(_call, n, x, ...) _call(n, x) _fei_21(_call, n + 1, __VA_ARGS__)
#define _fei_23(_call, n, x, ...) _call(n, x) _fei_22(_call, n + 1, __VA_ARGS__)
#define _fei_24(_call, n, x, ...) _call(n, x) _fei_23(_call, n + 1, __VA_ARGS__)
#define _fei_25(_call, n, x, ...) _call(n, x) _fei_24(_call, n + 1, __VA_ARGS__)
#define _fei_26(_call, n, x, ...) _call(n, x) _fei_25(_call, n + 1, __VA_ARGS__)
#define _fei_27(_call, n, x, ...) _call(n, x) _fei_26(_call, n + 1, __VA_ARGS__)
#define _fei_28(_call, n, x, ...) _call(n, x) _fei_27(_call, n + 1, __VA_ARGS__)
#define _fei_29(_call, n, x, ...) _call(n, x) _fei_28(_call, n + 1, __VA_ARGS__)
#define _fei_30(_call, n, x, ...) _call(n, x) _fei_29(_call, n + 1, __VA_ARGS__)
#define _fei_31(_call, n, x, ...) _call(n, x) _fei_30(_call, n + 1, __VA_ARGS__)
#define _fei_32(_call, n, x, ...) _call(n, x) _fei_31(_call, n + 1, __VA_ARGS__)
#define _fei_33(_call, n, x, ...) _call(n, x) _fei_32(_call, n + 1, __VA_ARGS__)
#define _fei_34(_call, n, x, ...) _call(n, x) _fei_33(_call, n + 1, __VA_ARGS__)
#define _fei_35(_call, n, x, ...) _call(n, x) _fei_34(_call, n + 1, __VA_ARGS__)
#define _fei_36(_call, n, x, ...) _call(n, x) _fei_35(_call, n + 1, __VA_ARGS__)
#define _fei_37(_call, n, x, ...) _call(n, x) _fei_36(_call, n + 1, __VA_ARGS__)
#define _fei_38(_call, n, x, ...) _call(n, x) _fei_37(_call, n + 1, __VA_ARGS__)
#define _fei_39(_call, n, x, ...) _call(n, x) _fei_38(_call, n + 1, __VA_ARGS__)
#define _fei_40(_call, n, x, ...) _call(n, x) _fei_39(_call, n + 1, __VA_ARGS__)

/**
 * Provide a for-each construct for variadic macros. Supports up
 * to 40 args.
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
#define CALL_MACRO_X_FOR_EACH(x, ...)                                          \
  _GET_NTH_ARG("ignored", ##__VA_ARGS__, _fe_40, _fe_39, _fe_38, _fe_37,       \
               _fe_36, _fe_35, _fe_34, _fe_33, _fe_32, _fe_31, _fe_30, _fe_29, \
               _fe_28, _fe_27, _fe_26, _fe_25, _fe_24, _fe_23, _fe_22, _fe_21, \
               _fe_20, _fe_19, _fe_18, _fe_17, _fe_16, _fe_15, _fe_14, _fe_13, \
               _fe_12, _fe_11, _fe_10, _fe_9, _fe_8, _fe_7, _fe_6, _fe_5,      \
               _fe_4, _fe_3, _fe_2, _fe_1, _fe_0)                              \
  (x, ##__VA_ARGS__)

#define CALL_MACRO_X_FOR_EACH_WITH_N(x, ...)                                   \
  _GET_NTH_ARG("ignored", ##__VA_ARGS__, _fei_40, _fei_39, _fei_38, _fei_37,   \
               _fei_36, _fei_35, _fei_34, _fei_33, _fei_32, _fei_31, _fei_30,  \
               _fei_29, _fei_28, _fei_27, _fei_26, _fei_25, _fei_24, _fei_23,  \
               _fei_22, _fei_21, _fei_20, _fei_19, _fei_18, _fei_17, _fei_16,  \
               _fei_15, _fei_14, _fei_13, _fei_12, _fei_11, _fei_10, _fei_9,   \
               _fei_8, _fei_7, _fei_6, _fei_5, _fei_4, _fei_3, _fei_2, _fei_1, \
               _fei_0)                                                         \
  (x, 0, ##__VA_ARGS__)

#define _NULL_IT(ptr) (ptr) = NULL;

#endif  // OIDCAGENT_MARCOS_H
