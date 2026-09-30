#include "Unicode.h"
#include "Test.h"

BOOLEAN TrUtf8ToCodePoint(
  IN CHAR8 CONST* Utf8String,
  IN UINTN Utf8StringSize,
  OUT UINTN* SequenceLength OPTIONAL,
  OUT UINT32* CodePoint OPTIONAL)
{
  if (Utf8String == NULL || Utf8StringSize < 1U) {
    return FALSE;
  }

  // 1-byte UTF-8: 0000 - 007F
  UINT8 C0 = (UINT8) Utf8String[0];
  if (C0 <= 0x7F) {
    TR_NONNULL_ASSIGN(SequenceLength, 1);
    TR_NONNULL_ASSIGN(CodePoint, (UINT32) C0);
    return TRUE;
  }

  if (Utf8StringSize < 2U) {
    return FALSE;
  }

  // 2-byte UTF-8: 0080 - 07FF
  if (C0 >= 0xC2 && C0 <= 0xDF) {
    UINT8 C1 = (UINT8) Utf8String[1];

    if (C1 < 0x80 || C1 > 0xBF) {
      return FALSE;
    }

    TR_NONNULL_ASSIGN(SequenceLength, 2);
    TR_NONNULL_ASSIGN(CodePoint,
      ((UINT32) (C0 & 0x1F) << 6) |
      ((UINT32) (C1 & 0x3F)     ));
    return TRUE;
  }

  if (Utf8StringSize < 3U) {
    return FALSE;
  }

  // 3-byte UTF-8: 0800 - FFFF
  if (C0 >= 0xE0 && C0 <= 0xEF) {
    UINT8 C1 = (UINT8) Utf8String[1];
    UINT8 C2 = (UINT8) Utf8String[2];

    if (C1 < 0x80 || C1 > 0xBF || C2 < 0x80 || C2 > 0xBF) {
      return FALSE;
    }

    // Reject overlong encodings: E0 80..9F
    if (C0 == 0xE0 && C1 < 0xA0) {
      return FALSE;
    }

    // Reject UTF-8 encodings of UTF-16 surrogates: ED A0..BF
    if (C0 == 0xED && C1 >= 0xA0) {
      return EFI_INVALID_PARAMETER;
    }

    TR_NONNULL_ASSIGN(SequenceLength, 3);
    TR_NONNULL_ASSIGN(CodePoint,
      ((UINT32) (C0 & 0x0F) << 12) |
      ((UINT32) (C1 & 0x3F) <<  6) |
      ((UINT32) (C2 & 0x3F)      ));
    return TRUE;
  }

  if (Utf8StringSize < 4U) {
    return FALSE;
  }

  // 4-byte UTF-8: 10000 - 10FFFF
  if (C0 >= 0xF0 && C0 <= 0xF4) {
    UINT8 C1 = (UINT8) Utf8String[1];
    UINT8 C2 = (UINT8) Utf8String[2];
    UINT8 C3 = (UINT8) Utf8String[3];

    if (C1 < 0x80 || C1 > 0xBF
        || C2 < 0x80 || C2 > 0xBF
        || C3 < 0x80 || C3 > 0xBF) {
      return FALSE;
    }

    // Reject overlong encodings: F0 80..8F
    if (C0 == 0xF0 && C1 < 0x90) {
      return FALSE;
    }

    // Reject code points above U+10FFFF: F4 90..BF
    if (C0 == 0xF4 && C1 > 0x8F) {
      return FALSE;
    }

    TR_NONNULL_ASSIGN(SequenceLength, 4);
    TR_NONNULL_ASSIGN(CodePoint,
      ((UINT32) (C0 & 0x07) << 18) |
      ((UINT32) (C1 & 0x3F) << 12) |
      ((UINT32) (C2 & 0x3F) <<  6) |
      ((UINT32) (C3 & 0x3F)      ));
    return TRUE;
  }

  return FALSE;
}

UINTN TrCodePointToUtf16(
  UINT32 CodePoint,
  CHAR16 Utf16Bytes[2] OPTIONAL)
  // (Yes, intention.)
{
  // Invalid Unicode value.
  if (CodePoint > 0x10FFFF ||
      (CodePoint >= 0xD800 && CodePoint <= 0xDFFF)) {
    return 0U;
  }

  // Basic Multilingual Plane (BMP).
  if (CodePoint <= 0xFFFF) {
    if (Utf16Bytes != NULL) {
      Utf16Bytes[0] = (CHAR16) CodePoint;
    }
    return 1U;
  }

  // Supplementary planes: add surrogate pair.
  CodePoint -= 0x10000;
  if (Utf16Bytes != NULL) {
    Utf16Bytes[0] = (CHAR16) (0xD800 + (CodePoint >> 10));
    Utf16Bytes[1] = (CHAR16) (0xDC00 + (CodePoint & 0x3FF));
  }
  return 2U;
}

// TODO: Tests that fail.
TR_TEST(TrUtf8UnicodeWith1CodePoint) {
  UINT32 CodePoint = 0U;
  UINTN SequenceLength = 0U;
  TR_TEST_ASSERT(TrUtf8ToCodePoint(
    "A", SIZEOF("A") - 1U, &SequenceLength, &CodePoint));
  TR_TEST_ASSERT_EQ(SequenceLength, 1);
  TR_TEST_ASSERT_EQ(CodePoint, 0x41U);
}

TR_TEST(TrUtf8UnicodeWith2CodePoint) {
  UINT32 CodePoint = 0U;
  UINTN SequenceLength = 0U;
  TR_TEST_ASSERT(TrUtf8ToCodePoint(
    "é", SIZEOF("é") - 1U, &SequenceLength, &CodePoint));
  TR_TEST_ASSERT_EQ(SequenceLength, 2);
  TR_TEST_ASSERT_EQ(CodePoint, 0xE9U);
}

TR_TEST(TrUtf8UnicodeWith3CodePoint) {
  UINT32 CodePoint = 0U;
  UINTN SequenceLength = 0U;
  TR_TEST_ASSERT(TrUtf8ToCodePoint(
    "小", SIZEOF("小") - 1U, &SequenceLength, &CodePoint));
  TR_TEST_ASSERT_EQ(SequenceLength, 3);
  TR_TEST_ASSERT_EQ(CodePoint, 0x5C0FU);
}

TR_TEST(TrUtf8UnicodeWith4CodePoint) {
  UINT32 CodePoint = 0U;
  UINTN SequenceLength = 0U;
  TR_TEST_ASSERT(TrUtf8ToCodePoint(
    "😀", SIZEOF("😀") - 1U, &SequenceLength, &CodePoint));
  TR_TEST_ASSERT_EQ(SequenceLength, 4);
  TR_TEST_ASSERT_EQ(CodePoint, 0x1F600U);
}

TR_TEST(TrAsciiToUtf16) {
  UINTN SequenceLength = 0;
  CHAR16 Utf16Bytes[2] = { 0x0 };
  TR_TEST_ASSERT(SequenceLength = TrCodePointToUtf16(
    0x41, Utf16Bytes)); // 0x41 = A
  TR_TEST_ASSERT_EQ(SequenceLength, 1);
  TR_TEST_ASSERT_EQ(Utf16Bytes[0], 0x41);
}

TR_TEST(Tr1CodePointToUtf16) {
  UINTN SequenceLength = 0;
  CHAR16 Utf16Bytes[2] = { 0x0 };
  TR_TEST_ASSERT(SequenceLength = TrCodePointToUtf16(
    0x5C0F, Utf16Bytes)); // 0x5C0F = 小
  TR_TEST_ASSERT_EQ(SequenceLength, 1);
  TR_TEST_ASSERT_EQ(Utf16Bytes[0], 0x5C0F);
}

TR_TEST(Tr2CodePointToUtf16) {
  UINTN SequenceLength = 0;
  CHAR16 Utf16Bytes[2] = { 0x0 };
  TR_TEST_ASSERT(SequenceLength = TrCodePointToUtf16(
    0x1F600, Utf16Bytes)); // 0x1F600 = 😀
  TR_TEST_ASSERT_EQ(SequenceLength, 2);
  TR_TEST_ASSERT_EQ(Utf16Bytes[0], 0xD83D);
  TR_TEST_ASSERT_EQ(Utf16Bytes[1], 0xDE00);
}
