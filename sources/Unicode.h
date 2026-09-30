#ifndef TR_UNICODE_H
#define TR_UNICODE_H

#include "EfiBase.h"

BOOLEAN TrUtf8ToCodePoint(
  IN CHAR8 CONST* Utf8String,
  IN UINTN Utf8StringSize,
  OUT UINTN* SequenceLength OPTIONAL,
  OUT UINT32* CodePoint OPTIONAL
);

UINTN TrCodePointToUtf16(
  IN UINT32 CodePoint,
  OUT CHAR16 Utf16Bytes[2] OPTIONAL
  // (Yes, intention.)
);

#endif // TR_UNICODE_H
