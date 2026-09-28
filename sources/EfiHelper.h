#ifndef TR_EFIHELPER_H
#define TR_EFIHELPER_H

#include "Efi.h"

extern EFI_SYSTEM_TABLE* gST;
extern EFI_BOOT_SERVICES* gBS;
extern EFI_RUNTIME_SERVICES* gRT;

#define TR_LOG_LITERAL(STRING)           \
  gST->ConOut->OutputString(gST->ConOut, \
    TR_L( STRING ) TR_CRLF);

VOID EFIAPI TrEfiInit(IN EFI_SYSTEM_TABLE* SystemTable);

#endif // TR_EFIHELPER_H
