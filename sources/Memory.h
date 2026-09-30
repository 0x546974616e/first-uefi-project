#ifndef TR_MEMORY_H
#define TR_MEMORY_H

#include "EfiBase.h"

#if !defined(__has_builtin)
#error __has_builtin is not defined.
#endif

#if __has_builtin(__builtin_memset)
// Au-delà de 256 bytes, memset() est appelée.
#define TR_MEMSET __builtin_memset
#else
#error __builtin_memset is not defined.
#endif

#if __has_builtin(__builtin_memset_inline)
#define TR_MEMSET_INLINE __builtin_memset_inline
#else
#error __builtin_memset_inline is not defined.
#endif

#if __has_builtin(__builtin_memcpy)
#define TR_MEMCPY __builtin_memcpy
#else
#error __builtin_memset is not defined.
#endif

#if __has_builtin(__builtin_memcpy_inline)
#define TR_MEMCPY_INLINE __builtin_memcpy_inline
#else
#error __builtin_memcpy_inline is not defined.
#endif

#if __has_builtin(__builtin_memcmp)
#define TR_MEMCMP __builtin_memcmp
#else
#error __builtin_memcmp is not defined.
#endif

VOID* EFIAPI TrMemorySet(
  OUT VOID* Destination,
  IN UINT8 Value,
  IN UINTN Size
);

VOID* EFIAPI TrMemoryCopy(
  OUT VOID* Destination,
  IN VOID CONST* Source,
  IN UINTN Length
);

INTN EFIAPI TrMemoryCompare(
  IN VOID CONST* BufferA,
  IN VOID CONST* BufferB,
  IN UINTN Length
);

INTN EFIAPI TrStringCompare(
  IN CHAR16 CONST* StringA,
  IN CHAR16 CONST* StringB,
  IN UINTN Length
);

#endif // TR_MEMORY_H
