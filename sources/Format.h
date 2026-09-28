#ifndef TR_FORMAT_H
#define TR_FORMAT_H

#include <stdarg.h>
#include "EfiBase.h"
#include "Helper.h"

typedef struct {
  VOID* Data;
  VOID (*Terminate)(IN VOID* Data);
  VOID (*OutCharacter)(
    IN VOID* Data,
    IN CHAR16 Character
  );
} TR_FORMAT_OUT_CALLBACK;

UINTN TrFormat(
  CHAR16* Buffer, UINTN N,
  CHAR16 const* Format, ...
);

UINTN TrFormatV(
  CHAR16* Buffer, UINTN N,
  CHAR16 const* Format, va_list Args
);

VOID TrFormatOutV(
  IN TR_FORMAT_OUT_CALLBACK* Callback,
  IN CHAR16 CONST* Format,
  VA_LIST Args
);

#endif // TR_FORMAT_H
