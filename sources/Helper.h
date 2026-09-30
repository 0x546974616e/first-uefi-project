#ifndef TR_HELPER_H
#define TR_HELPER_H

// Clang does not define `__null`.
#define NULL ((void*) 0)
#define SIZEOF sizeof
#define VOLATILE volatile
#define SIGNED signed
#define STATIC static
#define INLINE inline

#define VA_LIST va_list
#define VA_START va_start
#define VA_ARG va_arg
#define VA_END va_end

#define TR_DO(X) do { X } while(0)

#define TR_L(X) TR_CONCAT(u, X)
#define TR_CRLF TR_L("\r\n")

#define TR_ABS(a) (((a) < 0) ? (-(a)) : (a))
#define TR_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define TR_MIN(a, b) (((a) < (b)) ? (a) : (b))

#define TR_CONCAT_HELPER(X, Y) X ## Y
#define TR_CONCAT(X, Y) TR_CONCAT_HELPER(X, Y)
#define TR_STRINGIFY_HELPER(X) #X
#define TR_STRINGIFY(X) TR_STRINGIFY_HELPER(X)

/// Size of a static C-style array. Don't use on pointers!
#define TR_ARRAYSIZE(ARRAY) ((size_t) (sizeof(ARRAY) / sizeof(*(ARRAY))))
#define TR_STATIC_ASSERT(X) _Static_assert((X), "UEFI requirements")
#define TR_NONNULL_ASSIGN(V, X) TR_DO(if ((V) != NULL) { *(V) = (X); })
#define TR_VALUE_OR(VALUE, OR) (VALUE) ? (VALUE) : (OR)
#define TR_SATURATING_SUB(X, Y) ((X) > (Y) ? (X) - (Y) : 0)
#define TR_BIT_MASK(TYPE, N) (((TYPE)1 << (N)) - (TYPE)1)
#define TR_HAS_FLAGS(X, M) (((X) & (M)) == (M))

#define TR_ATTRIBUTE(X) __attribute__((X))
#define TR_ALIGN_VALUE(X) TR_ATTRIBUTE(align_value(X))
#define TR_ALIGNAS(X) TR_ATTRIBUTE(aligned(X))
#define TR_SECTION(X) TR_ATTRIBUTE(section(X))
#define TR_UNUSED TR_ATTRIBUTE(unused)

// NOTE:
// CHAR16 is a UEFI-defined type and not recognized as C-string type.
// Therefore, static analyser will not validate format-string arguments.
#if __SIZEOF_WCHAR_T__ == 0xDEADBEEF
#define TR_FMTARGS(POSITION) TR_ATTRIBUTE(format(printf, POSITION, POSITION+1))
#define TR_FMTLIST(POSITION) TR_ATTRIBUTE(format(printf, POSITION, 0))
#else
#define TR_FMTARGS(POSITION)
#define TR_FMTLIST(POSITION)
#endif

typedef float FLOAT;
typedef double DOUBLE;
typedef signed int INT;
typedef unsigned int UINT;
typedef signed long LONG;
typedef unsigned long ULONG;
typedef signed long long LONG_LONG;
typedef unsigned long long ULONG_LONG;
typedef long double LONG_DOUBLE;
typedef size_t SIZE_T;

#endif // TR_HELPER_H
