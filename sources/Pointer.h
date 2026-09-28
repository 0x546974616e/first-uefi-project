
// 12.5
// EFI_SIMPLE_POINTER_PROTOCOL
// 12.7
// EFI_ABSOLUTE_POINTER_PROTOCOL

#include "Efi.h"
#include "EfiHelper.h"

STATIC EFI_SIMPLE_POINTER_PROTOCOL* gSimplePointerProtocol = NULL;
STATIC EFI_GUID gEfiSimplePointerProtocolGuid =
  EFI_SIMPLE_POINTER_PROTOCOL_GUID;

STATIC VOID TrInitSimplePointer(VOID) {
  // This buffer is allocated with a call to the Boot Service
  // `gBS->AllocatePool()`. It is the caller’s responsibility
  // to call the Boot Service `gBS->FreePool()`.
  EFI_HANDLE* HandleBuffer;
  UINTN HandleCount = 0U;
  EFI_STATUS Status;

  Status = gBS->LocateHandleBuffer(
    ByProtocol, &gEfiSimplePointerProtocolGuid,
    NULL, &HandleCount, &HandleBuffer );

  if (Status != EFI_SUCCESS) {
    TR_LOG("Locate(SimplePointer) failed.");
    return;
  }

  if (HandleCount <= 0U) {
    TR_LOG("HandleCount <= 0U.");
    return;
  }

  // TODO TMP WIP
  // Status = gBS->HandleProtocol(
  //   HandleBuffer[0], &gEfiSimplePointerProtocolGuid,
  //   (VOID**) &gSimplePointerProtocol);
}
