#include "Print.h"
#include "EfiHelper.h"

#define TR_PRINT_BUFFER_SIZE 128

typedef struct {
  UINTN Cursor;
  CHAR16 Data[TR_PRINT_BUFFER_SIZE];
} TR_PRINT_BUFFER;

STATIC VOID TrPrintOutCharacter(
  IN VOID* Data,
  IN CHAR16 Character)
{
  TR_PRINT_BUFFER* Buffer = (TR_PRINT_BUFFER*) Data;
  if (Buffer->Cursor >= TR_PRINT_BUFFER_SIZE - 1U) {
    Buffer->Data[TR_PRINT_BUFFER_SIZE - 1U] = TR_L( '\0' );
    gST->ConOut->OutputString(gST->ConOut, Buffer->Data);
    Buffer->Cursor = 0U;
  }
  Buffer->Data[Buffer->Cursor++] = Character;
}

STATIC VOID TrPrintTerminate(
  IN VOID* Data)
{
  TR_PRINT_BUFFER* Buffer = (TR_PRINT_BUFFER*) Data;
  if (Buffer->Cursor > 0U) {
    UINTN Index = Buffer->Cursor < TR_PRINT_BUFFER_SIZE
      ? Buffer->Cursor : TR_PRINT_BUFFER_SIZE - 1U;
    Buffer->Data[Index] = TR_L( '\0' );
    gST->ConOut->OutputString(gST->ConOut, Buffer->Data);
  }
}

VOID TrPrint(
  CHAR16 const* Format, ...)
{
  TR_PRINT_BUFFER Buffer = { 0x0 };
  TR_FORMAT_OUT_CALLBACK Callback = {
    .Data = (VOID*) &Buffer,
    .Terminate = TrPrintTerminate,
    .OutCharacter = TrPrintOutCharacter,
  };

  VA_LIST Args;
  VA_START(Args, Format);
  (void) TrFormatOutV(&Callback, Format, Args);
  VA_END(Args);
}
