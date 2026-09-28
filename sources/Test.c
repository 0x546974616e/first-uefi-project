#include "EfiHelper.h"
#include "Print.h"
#include "Test.h"

TR_TEST_FUNCTION_POINTER_HELPER(
  NULL, gTrTestSectionStart,
  TR_TEST_SECTION_START
);

TR_TEST_FUNCTION_POINTER_HELPER(
  NULL, gTrTestSectionEnd,
  TR_TEST_SECTION_END
);

VOID TrRunTests(VOID) {
  TR_UNIT_TEST CONST* X = &gTrTestSectionStart;
  for (++X; X && X < &gTrTestSectionEnd; ++X) {
    if (X && X->Function && X->Name) {
      TR_UNIT_TEST_CODE Code = TrUnitTestSuccess;
      TR_PRINTLN("Running %ls...", X->Name);
      (*X->Function)(&Code);
      TR_PRINTLN("%ls: %ls\r\n", X->Name,
        Code == TrUnitTestSuccess
          ? TR_L( "Success" ) : TR_L( "Failed" ));
    }
  }
}

// Examples

TR_TEST(TrAlwaysSuccessful) {
  TR_TEST_ASSERT(TRUE);
}

TR_TEST(TrAlwaysFails) {
  TR_TEST_ASSERT(1 == 2);
  TR_LOG_LITERAL("!!! Unreachable !!!");
}
