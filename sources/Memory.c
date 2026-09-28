#include "EfiHelper.h"
#include "Memory.h"

void* memset(void* s , int c, size_t n) {
  gBS->SetMem(s, n, (UINT8) c);
  return s;
}

VOID* EFIAPI TrMemorySet(
  OUT VOID* Destination,
  IN UINT8 Value,
  IN UINTN Size)
{
  gBS->SetMem(Destination, Size, Value);
  return Destination;
}

VOID* EFIAPI TrMemoryCopy(
  OUT VOID* Destination,
  IN VOID CONST* Source,
  IN UINTN Length)
{
  gBS->CopyMem(Destination, (VOID*) Source, Length);
  return Destination;
}

INTN EFIAPI TrMemoryCompare(
  IN VOID CONST* BufferA,
  IN VOID CONST* BufferB,
  IN UINTN Length)
{
  if ((Length == 0) || (BufferA == BufferB)) {
    return 0;
  }

  // TODO: Smarter comparison by 4 or 8 bytes.
  // if (sizeof (UINTN) == sizeof (UINT64)) {} else {}
  volatile UINT8 CONST* BufferA8 = (UINT8 CONST*) BufferA;
  volatile UINT8 CONST* BufferB8 = (UINT8 CONST*) BufferB;
  while ((--Length != 0) && *(BufferA8++) != *(BufferB8++));
  return (INTN) (*BufferA8 - *BufferB8);
}
