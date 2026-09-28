#include "Efi.h"
#include "Graphics.h"
#include "Atlas.inl"

// #define TR_WIDTH 400
// #define TR_HEIGHT 600
// static EFI_GRAPHICS_OUTPUT_BLT_PIXEL Screen[TR_WIDTH * TR_HEIGHT];

// https://axleos.com/an-irc-client-in-your-motherboard/
static EFI_GRAPHICS_OUTPUT_PROTOCOL* pGO = NULL;
static EFI_GUID gEfiGraphicsOutputProtocolGuid =
  EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

// NOTE:
//   EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.PixelsPerScanLine
//    ...is the number of pixels between the start of one framebuffer
//       row and the start of the next row.
//
// EXAMPLE:
//   EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.HorizontalResolution = 1920
//   EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.VerticalResolution   = 1080
//   EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.PixelsPerScanLine    = 2048
//     Framebuffer:
//     ┌──────────────────────────────────┬──────────────────┐
//     │          Visible Screen          │ Padding (unused) │
//     │            1920 × 1080           │    128 × 1080    │
//     │                                  │                  │
//     │                                  │                  │
//     │                                  │                  │
//     │                                  │                  │
//     │                                  │                  │
//     └──────────────────────────────────┴──────────────────┘

static void dadafafa(void) {}
