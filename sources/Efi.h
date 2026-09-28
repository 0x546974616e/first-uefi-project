#ifndef TR_EFI_H
#define TR_EFI_H

#include "Helper.h"
#include "EfiBase.h"

#pragma region Type

/// Common UEFI Data Types
/// (UEFI Spec 2.11 section 2.3.1)
typedef UINTN EFI_STATUS;
typedef VOID* EFI_HANDLE;
typedef VOID* EFI_EVENT;

/// Physical Address
/// (UEFI Spec 2.11 section 7.2.1)
typedef UINT64 EFI_PHYSICAL_ADDRESS;

#pragma region Code

/// EFI_STATUS codes
/// (UEFI Spec 2.11 appendix D)
#define EFI_SUCCESS 0
#define EFI_LOAD_ERROR 1
#define EFI_INVALID_PARAMETER 2
#define EFI_UNSUPPORTED 3
#define EFI_BAD_BUFFER_SIZE 4
#define EFI_BUFFER_TOO_SMALL 5
#define EFI_NOT_READY 6
#define EFI_DEVICE_ERROR 7
#define EFI_WRITE_PROTECTED 8
#define EFI_OUT_OF_RESOURCES 9
#define EFI_VOLUME_CORRUPTED 10
#define EFI_VOLUME_FULL 11
#define EFI_NO_MEDIA 12
#define EFI_MEDIA_CHANGED 13
#define EFI_NOT_FOUND 14
#define EFI_ACCESS_DENIED 15
#define EFI_NO_RESPONSE 16
#define EFI_NO_MAPPING 17
#define EFI_TIMEOUT 18
#define EFI_NOT_STARTED 19
#define EFI_ALREADY_STARTED 20
#define EFI_ABORTED 21
#define EFI_ICMP_ERROR 22
#define EFI_TFTP_ERROR 23
#define EFI_PROTOCOL_ERROR 24
#define EFI_INCOMPATIBLE_VERSION 25
#define EFI_SECURITY_VIOLATION 26
#define EFI_CRC_ERROR 27
#define EFI_END_OF_MEDIA 28
#define EFI_END_OF_FILE 31
#define EFI_INVALID_LANGUAGE 32
#define EFI_COMPROMISED_DATA 33
#define EFI_IP_ADDRESS_CONFLICT 34
#define EFI_HTTP_ERROR 35

#pragma region GUID

/// Simple Pointer Protocol
/// (UEFI Spec 2.11 section 12.5.1)
#define EFI_SIMPLE_POINTER_PROTOCOL_GUID \
  { 0x31878c87, 0xb75, 0x11d5,           \
    { 0x9a, 0x4f, 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d } }

/// Graphics Output Protocol
/// (UEFI Spec 2.11 section 12.9.2)
#define EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID \
  { 0x9042a9de, 0x23dc, 0x4a38,           \
    { 0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a } }

#pragma region Forward

/// Forward structure declaration
#define DECLARE_STRUCT(X) typedef struct X X
DECLARE_STRUCT(EFI_GRAPHICS_OUTPUT_PROTOCOL);
DECLARE_STRUCT(EFI_SIMPLE_POINTER_PROTOCOL);
DECLARE_STRUCT(EFI_SIMPLE_TEXT_INPUT_PROTOCOL);
DECLARE_STRUCT(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL);
#undef DECLARE_STRUCT

#pragma region Enum

/// Memory Type
/// (UEFI Spec 2.11 section 7.2.1)
typedef enum {
  EfiReservedMemoryType,
  EfiLoaderCode,
  EfiLoaderData,
  EfiBootServicesCode,
  EfiBootServicesData,
  EfiRuntimeServicesCode,
  EfiRuntimeServicesData,
  EfiConventionalMemory,
  EfiUnusableMemory,
  EfiACPIReclaimMemory,
  EfiACPIMemoryNVS,
  EfiMemoryMappedIO,
  EfiMemoryMappedIOPortSpace,
  EfiPalCode,
  EfiPersistentMemory,
  EfiUnacceptedMemoryType,
  EfiMaxMemoryType,
} EFI_MEMORY_TYPE;

/// Locate Search Type
/// (UEFI Spec 2.11 section 7.3.6)
typedef enum {
  AllHandles,
  ByRegisterNotify,
  ByProtocol,
} EFI_LOCATE_SEARCH_TYPE;

/// Reset System
/// (UEFI Spec 2.11 section 8.5.1.1)
typedef enum {
  EfiResetCold,
  EfiResetWarm,
  EfiResetShutdown,
  EfiResetPlatformSpecific,
} EFI_RESET_TYPE;

/// Bit Block Transfer - Operation
/// (UEFI Spec 2.11 section 12.9.2.3)
typedef enum {
  EfiBltVideoFill,
  EfiBltVideoToBltBuffer,
  EfiBltBufferToVideo,
  EfiBltVideoToVideo,
  EfiGraphicsOutputBltOperationMax,
} EFI_GRAPHICS_OUTPUT_BLT_OPERATION;

#pragma region Struct

/// Gobally Unique Identifier
/// (UEFI Spec 2.11 section 7.3.2)
/// (UEFI Spec 2.11 appendix A)
typedef struct {
  UINT32 Data1;
  UINT16 Data2;
  UINT16 Data3;
  UINT8 Data4[8];
} EFI_GUID;

/// Simple Text Input Protocol
/// (UEFI Spec 2.11 section 12.3.3)
typedef struct {
  UINT16 ScanCode;
  CHAR16 UnicodeChar;
} EFI_INPUT_KEY;

/// Graphics Output - Pixel Bitmask
/// (UEFI Spec 2.11 section 12.9.2)
typedef struct {
  UINT32 RedMask;
  UINT32 GreenMask;
  UINT32 BlueMask;
  UINT32 ReservedMask;
} EFI_PIXEL_BITMASK;

/// Graphics Output - Pixel Format
/// (UEFI Spec 2.11 section 12.9.2)
typedef enum {
  PixelRedGreenBlueReserved8BitPerColor,
  PixelBlueGreenRedReserved8BitPerColor,
  PixelBitMask,
  PixelBltOnly,
  PixelFormatMax,
} EFI_GRAPHICS_PIXEL_FORMAT;

/// Graphics Output - Mode Information
/// (UEFI Spec 2.11 section 12.9.2)
typedef struct {
  UINT32 Version;
  UINT32 HorizontalResolution;
  UINT32 VerticalResolution;
  EFI_GRAPHICS_PIXEL_FORMAT PixelFormat;
  EFI_PIXEL_BITMASK PixelInformation;
  UINT32 PixelsPerScanLine;
} EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;

/// Graphics Output - Mode
/// (UEFI Spec 2.11 section 12.9.2)
typedef struct {
  UINT32 MaxMode;
  UINT32 Mode;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
  UINTN SizeOfInfo;
  EFI_PHYSICAL_ADDRESS FrameBufferBase;
  UINTN FrameBufferSize;
} EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE;

/// Bit Block Transfer - Pixel
/// (UEFI Spec 2.11 section 12.9.2.3)
typedef struct {
  UINT8 Blue;
  UINT8 Green;
  UINT8 Red;
  UINT8 Reserved;
} EFI_GRAPHICS_OUTPUT_BLT_PIXEL;

#pragma region Function

/// EFI_BOOT_SERVICES.AllocatePool()
/// (UEFI Spec 2.11 section 7.2.4)
typedef
EFI_STATUS (EFIAPI *EFI_ALLOCATE_POOL) (
  IN EFI_MEMORY_TYPE PoolType,
  IN UINTN Size,
  OUT VOID** Buffer
);

/// EFI_BOOT_SERVICES.FreePool()
/// (UEFI Spec 2.11 section 7.2.5)
typedef
EFI_STATUS (EFIAPI *EFI_FREE_POOL) (
  IN VOID* Buffer
);

/// EFI_BOOT_SERVICES.LocateHandleBuffer()
/// (UEFI Spec 2.11 section 7.3.15)
typedef
EFI_STATUS (EFIAPI *EFI_LOCATE_HANDLE_BUFFER) (
  IN EFI_LOCATE_SEARCH_TYPE SearchType,
  IN EFI_GUID* Protocol OPTIONAL,
  IN VOID* SearchKey OPTIONAL,
  OUT UINTN* NoHandles,
  OUT EFI_HANDLE** Buffer
);

/// EFI_BOOT_SERVICES.LocateProtocol()
/// (UEFI Spec 2.11 section 7.3.16)
typedef
EFI_STATUS (EFIAPI *EFI_LOCATE_PROTOCOL) (
  IN EFI_GUID* Protocol,
  IN VOID* Registration OPTIONAL,
  OUT VOID** Interface
);

/// EFI_BOOT_SERVICES.Stall()
/// (UEFI Spec 2.11 section 7.5.2)
typedef
EFI_STATUS (EFIAPI *EFI_STALL) (
  IN UINTN Microseconds
);

/// EFI_BOOT_SERVICES.CopyMem()
/// (UEFI Spec 2.11 section 7.5.3)
typedef
VOID (EFIAPI *EFI_COPY_MEM) (
  IN VOID *Destination,
  IN VOID *Source,
  IN UINTN Length
);

/// EFI_BOOT_SERVICES.SetMem()
/// (UEFI Spec 2.11 section 7.5.4)
typedef
VOID (EFIAPI *EFI_SET_MEM) (
  IN VOID *Buffer,
  IN UINTN Size,
  IN UINT8 Value
);

/// EFI_RESET_SYSTEM.ResetSystem()
/// (UEFI Spec 2.11 section 8.5.1.1)
typedef
VOID (EFIAPI *EFI_RESET_SYSTEM) (
  IN EFI_RESET_TYPE ResetType,
  IN EFI_STATUS ResetStatus,
  IN UINTN DataSize,
  IN VOID* ResetData OPTIONAL
);

/// EFI_SIMPLE_TEXT_INPUT_PROTOCOL.ReadKeyStroke()
/// (UEFI Spec 2.11 section 12.3.3)
typedef
EFI_STATUS (EFIAPI *EFI_INPUT_READ_KEY) (
  IN EFI_SIMPLE_TEXT_INPUT_PROTOCOL* This,
  OUT EFI_INPUT_KEY* Key
);

/// EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString()
/// (UEFI Spec 2.11 section 12.4.3)
typedef
EFI_STATUS (EFIAPI *EFI_TEXT_STRING) (
  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This,
  IN CHAR16* String
);

/// EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.ClearScreen()
/// (UEFI Spec 2.11 section 12.4.8)
typedef
EFI_STATUS (EFIAPI *EFI_TEXT_CLEAR_SCREEN) (
  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This
);

/// EFI_GRAPHICS_OUTPUT_PROTOCOL.Blt()
/// (UEFI Spec 2.11 section 12.9.2.3)
typedef
EFI_STATUS (EFIAPI *EFI_GRAPHICS_OUTPUT_PROTOCOL_BLT) (
  IN EFI_GRAPHICS_OUTPUT_PROTOCOL* This,
  IN OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL* BltBuffer OPTIONAL,
  IN EFI_GRAPHICS_OUTPUT_BLT_OPERATION BltOperation,
  IN UINTN SourceX,
  IN UINTN SourceY,
  IN UINTN DestinationX,
  IN UINTN DestinationY,
  IN UINTN Width,
  IN UINTN Height,
  IN UINTN Delta OPTIONAL
);

#pragma region Protocol

/// Simple Text Input Protocol
/// (UEFI Spec 2.11 section 12.3.1)
struct EFI_SIMPLE_TEXT_INPUT_PROTOCOL {
  TODO* Reset;
  EFI_INPUT_READ_KEY ReadKeyStroke;
  TODO* WaitForKey;
};

/// Simple Text Output Protocol
/// (UEFI Spec 2.11 section 12.4.1)
struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
  TODO* Reset;
  EFI_TEXT_STRING OutputString;
  TODO* TestString;
  TODO* QueryMode;
  TODO* SetMode;
  TODO* SetAttribute;
  EFI_TEXT_CLEAR_SCREEN ClearScreen;
  TODO* SetCursorPosition;
  TODO* EnableCursor;
  TODO* Mode;
};

/// Simple Pointer Protocol
/// (UEFI Spec 2.11 section 12.5.1)
struct EFI_SIMPLE_POINTER_PROTOCOL {
  TODO* Reset;
  TODO* GetState;
  EFI_EVENT WaitForInput;
  TODO* Mode;
};

/// Graphics Output Protocol
/// (UEFI Spec 2.11 section 12.9.2)
struct EFI_GRAPHICS_OUTPUT_PROTOCOL {
  TODO* QueryMode;
  TODO* SetMode;
  EFI_GRAPHICS_OUTPUT_PROTOCOL_BLT Blt;
  EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* Mode;
};

#pragma region Table

/// EFI Table Header
/// (UEFI Spec 2.11 section 4.2)
typedef struct {
  UINT64 Signature;
  UINT32 Revision;
  UINT32 HeaderSize;
  UINT32 CRC32;
  UINT32 Reserved;
} EFI_TABLE_HEADER;

/// EFI Runtime Services Table
/// (UEFI Spec 2.11 section 4.5.1)
typedef struct {
  EFI_TABLE_HEADER Hdr;
  TODO* GetTime;
  TODO* SetTime;
  TODO* GetWakeupTime;
  TODO* SetWakeupTime;
  TODO* SetVirtualAddressMap;
  TODO* ConvertPointer;
  TODO* GetVariable;
  TODO* GetNextVariableName;
  TODO* SetVariable;
  TODO* GetNextHighMonotonicCount;
  EFI_RESET_SYSTEM ResetSystem;
  TODO* UpdateCapsule;
  TODO* QueryCapsuleCapabilities;
  TODO* QueryVariableInfo;
} EFI_RUNTIME_SERVICES;

/// EFI Boot Services Table
/// (UEFI Spec 2.11 section 4.4.1)
typedef struct {
  EFI_TABLE_HEADER Hdr;
  TODO* RaiseTPL;
  TODO* RestoreTPL;
  TODO* AllocatePages;
  TODO* FreePages;
  TODO* GetMemoryMap;
  EFI_ALLOCATE_POOL AllocatePool;
  EFI_FREE_POOL FreePool;
  TODO* CreateEvent;
  TODO* SetTimer;
  TODO* WaitForEvent;
  TODO* SignalEvent;
  TODO* CloseEvent;
  TODO* CheckEvent;
  TODO* InstallProtocolInterface;
  TODO* ReinstallProtocolInterface;
  TODO* UninstallProtocolInterface;
  TODO* HandleProtocol;
  VOID* Reserved;
  TODO* RegisterProtocolNotify;
  TODO* LocateHandle;
  TODO* LocateDevicePath;
  TODO* InstallConfigurationTable;
  TODO* LoadImage;
  TODO* StartImage;
  TODO* Exit;
  TODO* UnloadImage;
  TODO* ExitBootServices;
  TODO* GetNextMonotonicCount;
  EFI_STALL Stall;
  TODO* SetWatchdogTimer;
  TODO* ConnectController;
  TODO* DisconnectController;
  TODO* OpenProtocol;
  TODO* CloseProtocol;
  TODO* OpenProtocolInformation;
  TODO* ProtocolsPerHandle;
  EFI_LOCATE_HANDLE_BUFFER LocateHandleBuffer;
  EFI_LOCATE_PROTOCOL LocateProtocol;
  TODO* InstallMultipleProtocolInterfaces;
  TODO* UninstallMultipleProtocolInterfaces;
  TODO* CalculateCrc32;
  EFI_COPY_MEM CopyMem;
  EFI_SET_MEM SetMem;
  TODO* CreateEventEx;
} EFI_BOOT_SERVICES;

/// EFI System Table
/// (UEFI Spec 2.11 section 4.3)
typedef struct {
  EFI_TABLE_HEADER Hdr;
  TODO* FirmwareVendor;
  UINT32 FirmwareRevision;
  TODO* ConsoleInHandle;
  EFI_SIMPLE_TEXT_INPUT_PROTOCOL* ConIn;
  TODO* ConsoleOutHandle;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* ConOut;
  TODO* StandardErrorHandle;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* StdErr;
  EFI_RUNTIME_SERVICES* RuntimeServices;
  EFI_BOOT_SERVICES* BootServices;
  UINTN NumberOfTableEntries;
  TODO* ConfigurationTable;
} EFI_SYSTEM_TABLE;

#endif // TR_EFI_H
