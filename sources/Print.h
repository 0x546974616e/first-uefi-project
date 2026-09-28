#ifndef TR_PRINT_H
#define TR_PRINT_H

#include "Format.h"

#define TR_PRINT(FORMAT, ...) \
  TrPrint(TR_L(FORMAT), __VA_ARGS__)

#define TR_PRINTLN(FORMAT, ...) \
  TrPrint(TR_L(FORMAT) TR_CRLF, __VA_ARGS__)

VOID TrPrint(
  CHAR16 const* Format, ...
);

#endif // TR_PRINT_H
