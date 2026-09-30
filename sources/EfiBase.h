#ifndef TR_EFIBASE_H
#define TR_EFIBASE_H

#include "Helper.h"
#include <stddef.h> // wchar_t

/// x86_64 Microsoft calling convention
#define EFIAPI __attribute__((ms_abi))

/// Modifiers for Common UEFI Data Types
/// (UEFI Spec 2.11 section 2.3.1)
#define IN
#define OUT
#define OPTIONAL
#define CONST const

/// Common UEFI Data Types
/// (UEFI Spec 2.11 section 2.3.1)
typedef signed char INT8;
typedef unsigned char UINT8;
typedef signed short INT16;
typedef unsigned short UINT16;
typedef signed int INT32;
typedef unsigned int UINT32;
typedef signed long long INT64;
typedef unsigned long long UINT64;
typedef signed long long INTN;
typedef unsigned long long UINTN;
typedef unsigned char BOOLEAN;
typedef void VOID, TODO;

TR_STATIC_ASSERT(sizeof(UINT8) == 1);
TR_STATIC_ASSERT(sizeof(UINT16) == 2);
TR_STATIC_ASSERT(sizeof(UINT32) == 4);
TR_STATIC_ASSERT(sizeof(UINT64) == 8);
TR_STATIC_ASSERT(sizeof(UINTN) == 8);

/// Strings are stored in the UCS-2 encoding format as
/// defined by Unicode 2.1 and ISO/IEC 10646 standards.
typedef wchar_t CHAR16;
typedef char CHAR8;

#if __SIZEOF_WCHAR_T__ != 2
#  error __SIZEOF_WCHAR_T__ != 2
#endif

TR_STATIC_ASSERT(sizeof(CHAR16) == 2);
TR_STATIC_ASSERT(sizeof(CHAR8) == 1);
TR_STATIC_ASSERT(sizeof(u'A') == 2);
TR_STATIC_ASSERT(sizeof(L'A') == 2);
TR_STATIC_ASSERT(sizeof(u"A") == 4);
TR_STATIC_ASSERT(sizeof(L"A") == 4);

#define TRUE ((BOOLEAN) (1==1))
#define FALSE ((BOOLEAN) (0==1))

#endif // TR_EFIBASE_H
