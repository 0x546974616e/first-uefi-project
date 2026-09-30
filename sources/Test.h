#ifndef TR_TEST_H
#define TR_TEST_H

#include "Helper.h"
#include "Print.h"
#include "Efi.h"

#if __clang_major__ != 19 || __clang_minor__ != 1
#error TrTest has only been tested with Clang 19.1.7
#endif

#define TR_TEST_SECTION test
#define TR_TEST_MAKE_SECTION(SUFFIX) \
  TR_STRINGIFY(TR_CONCAT(TR_TEST_SECTION, SUFFIX))

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdollar-in-identifier-extension"
#define TR_TEST_SECTION_START  TR_TEST_MAKE_SECTION($a)
#define TR_TEST_SECTION_MIDDLE TR_TEST_MAKE_SECTION($m)
#define TR_TEST_SECTION_END    TR_TEST_MAKE_SECTION($z)
#pragma clang diagnostic pop

#define TR_TEST_FUNCTION(X) void X( \
  OUT TR_UNIT_TEST_CODE* TR_UNUSED _TrTestReturnCode)
#define TR_TEST_FUNCTION_POINTER_HELPER(FUNCTION, IDENTIFIER, SECTION) \
  TR_UNIT_TEST IDENTIFIER TR_SECTION(SECTION) = {                      \
    .Name = TR_L(TR_STRINGIFY(FUNCTION)), .Function = FUNCTION }

#define TR_TEST_FUNCTION_POINTER(FUNCTION)      \
  TR_TEST_FUNCTION_POINTER_HELPER(              \
    FUNCTION, TR_CONCAT(FUNCTION, __COUNTER__), \
    TR_TEST_SECTION_MIDDLE)

#define TR_TEST_ASSERT(X) do {                 \
    if (!(X)) {                                \
      *_TrTestReturnCode = TrUnitTestFailure;  \
      TR_LPRINTLN("  Assertion failed: [%ls]", \
        TR_L(TR_STRINGIFY(X)));                \
      return;                                  \
    }                                          \
  } while(0)

#define TR_TEST_ASSERT_EQ(X, Y) do {                   \
    if ((X) != (Y)) {                                  \
      *_TrTestReturnCode = TrUnitTestFailure;          \
      TR_LPRINTLN("  Assertion failed: %ls != %ls",    \
        TR_L(TR_STRINGIFY(X)), TR_L(TR_STRINGIFY(Y))); \
      return;                                          \
    }                                                  \
  } while(0)

#define TR_TEST_FAILED() do {               \
    *_TrTestReturnCode = TrUnitTestFailure; \
    return;                                 \
  } while(0)

#define TR_TEST(X)             \
  TR_TEST_FUNCTION(X);         \
  TR_TEST_FUNCTION_POINTER(X); \
  TR_TEST_FUNCTION(X)

typedef enum {
  TrUnitTestSuccess = 0,
  TrUnitTestFailure = 1,
} TR_UNIT_TEST_CODE;

typedef struct {
  CHAR16 CONST* Name;
  TR_TEST_FUNCTION((*CONST Function));
} TR_UNIT_TEST;

VOID TrRunTests(VOID);

#endif // TR_TEST_H
