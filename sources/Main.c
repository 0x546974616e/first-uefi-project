#include "Efi.h"
#include "Helper.h"
#include "EfiHelper.h"
#include "Memory.h"
#include "MicroUi.h"
#include "Test.h"
#include "Print.h"

#include "Atlas.inl"

STATIC UINTN String16Length(CHAR16 CONST* String) {
  SIZE_T Length = 0;
  while (String[Length] != TR_L( '\0' )) Length++;
  return Length;
}


int r_get_text_width(const CHAR16 *text, int len) {
  int res = 0;
  for (CHAR16 const* p = text; *p && len--; p++) {
    UINT8 byte = (UINT8) *p;
    if ((byte & 0xC0) == 0x80) { continue; }
    int chr = TR_MIN(byte, 127);
    res += Atlas[ATLAS_FONT + chr].w;
  }
  return res;
}


int r_get_text_height(void) {
  return 18;
}



#define TW 100
static int text_width(mu_Font font, const CHAR16 *text, int len) {
  if (len == -1) { len = (int) String16Length(text); }
  return r_get_text_width(text, len);
  // return TW;
}

#define TH 18
static int text_height(mu_Font font) {
  // return TH;
  return r_get_text_height();
}

static EFI_GUID gEfiGraphicsOutputProtocolGuid =
  EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

static mu_Context ctx;
static EFI_GRAPHICS_OUTPUT_PROTOCOL* pGO = NULL;
static mu_Rect clip = { 0, 0, 0x1000000, 0x1000000 };
#define WW 400
#define HH 600
static EFI_GRAPHICS_OUTPUT_BLT_PIXEL Screen[WW * HH];
void push_quad(mu_Rect rect, mu_Color color) {
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL Pixel = {
    .Red = color.r, .Green = color.g, .Blue = color.b,
    .Reserved = 0
  };

  for (int y = rect.y; y < rect.y + rect.h; ++y) {
    for (int x = rect.x; x < rect.x + rect.w; ++x) {
      if (0 <= y && y < HH && 0 <= x && x < WW) {
        if (clip.x <= x && x < clip.x + clip.w) {
          if (clip.y <= y && y < clip.y + clip.h) {
            Screen[y * WW + x] = Pixel;
          }
        }
      }
    }
  }
}

void draw_atlas_1(mu_Vec2 d, mu_Rect r) {
  for (int y = 0; y < r.h; ++y) {
    for (int x = 0; x < r.w; ++x) {
      int xx = d.x + x, yy = d.y + y;
      if (0 <= yy && yy < HH && 0 <= xx && xx < WW) {
        UINT8 b = AtlasTexture[(r.y + y) * ATLAS_WIDTH + (r.x + x)];
        EFI_GRAPHICS_OUTPUT_BLT_PIXEL cc = { b, b, b, 0U };
        Screen[yy * WW + xx] = cc;
      }
    }
  }
}

void draw_atlas(mu_Vec2 p) {
  mu_Vec2 d = p;
  int m = sizeof(Atlas) / sizeof(*Atlas);
  for (int i = 0; i < m; ++i) {
    mu_Rect s = Atlas[i];
    draw_atlas_1(d, s);
    d.x += s.w + 1;
    if ((p.x + 50) <= d.x) {
      d.x = p.x;
      d.y += 18;
    }
  }

  // for (UINTN i = 0U; i < 120; ++i) {
  //   mu_Rect s = Atlas[i];
  //   if (s.w == 0 && s.h == 0) continue;
  //   draw_atlas_1(d, s);
  //   d.x += s.w + 5;
  //   if (d.x + 20 >= WW) {
  //     d.x = p.x;
  //     d.y += 18;
  //   }
  // }
}

void draw_atlas_texture(mu_Vec2 p) {
  for (int y = 0; y < ATLAS_HEIGHT; ++y) {
    for (int x = 0; x < ATLAS_WIDTH; ++x) {
      int yy = y + p.y, xx = x + p.x;
      if (0 <= yy && yy < HH && 0 <= xx && xx < WW) {
        UINT8 b = AtlasTexture[y * ATLAS_WIDTH + x];
        EFI_GRAPHICS_OUTPUT_BLT_PIXEL cc = { b, b, b, 0U };
        Screen[yy * WW + xx] = cc;
      }
    }
  }
}

void r_draw_text_1(mu_Vec2 d, mu_Rect r, mu_Color c) {
  for (int y = 0; y < r.h; ++y) {
    for (int x = 0; x < r.w; ++x) {
      int xx = d.x + x, yy = d.y + y;
      if (0 <= yy && yy < HH && 0 <= xx && xx < WW) {
        if (clip.x <= xx && xx < clip.x + clip.w) {
          if (clip.y <= yy && yy < clip.y + clip.h) {
            UINT8 b = AtlasTexture[(r.y + y) * ATLAS_WIDTH + (r.x + x)];
            // EFI_GRAPHICS_OUTPUT_BLT_PIXEL cc = { b, b, b, 0U };
            EFI_GRAPHICS_OUTPUT_BLT_PIXEL cc = { c.b, c.g, c.r, 0U };
            EFI_GRAPHICS_OUTPUT_BLT_PIXEL ss = Screen[yy * WW + xx];
            EFI_GRAPHICS_OUTPUT_BLT_PIXEL nn = {
              (cc.Blue * b) / 255 + (ss.Blue * (255 - b)) / 255,
              (cc.Green * b) / 255 + (ss.Green * (255 - b)) / 255,
              (cc.Red * b) / 255 + (ss.Red * (255 - b)) / 255,
              0U
            };
            Screen[yy * WW + xx] = nn;
          }
        }
      }
    }
  }
}

static CHAR16 g_dadafafa[1000] = { 0x0 };
static UINTN g_dadafafagaga = 0U;

static BOOLEAN IsPrint(CHAR16 c) {
  int x = ((int) c - ' ');
  return 0 <= x && x < 95;
}

void r_draw_text(CHAR16 const* text, mu_Vec2 pos, mu_Color color) {
  for (CHAR16 const* p = text; *p != TR_L( '\0' ); ++p) {
    UINT8 b = (UINT8) *p;
    if ((b & 0xc0) == 0x80) { continue; }

    UINT8 chr = TR_MIN(b, 127);
    // static const CHAR16 s_digits[16] = TR_L( "0123456789ABCDEF" );
    // if (g_dadafafagaga < TR_ARRAYSIZE(g_dadafafa))
    //   g_dadafafa[g_dadafafagaga++] = IsPrint(*p) ? *p : TR_L( '?' );
    // if (g_dadafafagaga < TR_ARRAYSIZE(g_dadafafa))
    //   g_dadafafa[g_dadafafagaga++] = (CHAR16) chr;
    // if (g_dadafafagaga < TR_ARRAYSIZE(g_dadafafa))
    //   g_dadafafa[g_dadafafagaga++] = TR_L( '(' );
    // if (g_dadafafagaga < TR_ARRAYSIZE(g_dadafafa))
    //   g_dadafafa[g_dadafafagaga++] = s_digits[((*p) >> 12) & 0xF];
    // if (g_dadafafagaga < TR_ARRAYSIZE(g_dadafafa))
    //   g_dadafafa[g_dadafafagaga++] = s_digits[((*p) >> 8) & 0xF];
    // if (g_dadafafagaga < TR_ARRAYSIZE(g_dadafafa))
    //   g_dadafafa[g_dadafafagaga++] = s_digits[((*p) >> 4) & 0xF];
    // if (g_dadafafagaga < TR_ARRAYSIZE(g_dadafafa))
    //   g_dadafafa[g_dadafafagaga++] = s_digits[(*p) & 0xF];
    // if (g_dadafafagaga < TR_ARRAYSIZE(g_dadafafa))
    //   g_dadafafa[g_dadafafagaga++] = TR_L( ')' );

    mu_Rect s = Atlas[ATLAS_FONT + chr];
    r_draw_text_1(pos, s, color);
    pos.x += s.w;
  }

  // for (CHAR16 const* p = TR_L( ";\r\n" ); *p
  //     && g_dadafafagaga < TR_ARRAYSIZE(g_dadafafa); ++p) {
  //   g_dadafafa[g_dadafafagaga++] = *p;
  // }
}

void r_draw_icon(int id, mu_Rect rect, mu_Color color) {
  mu_Rect s = Atlas[id];
  int x = rect.x + (rect.w - s.w) / 2;
  int y = rect.y + (rect.h - s.h) / 2;
  r_draw_text_1((mu_Vec2) { x, y }, s, color);
  // push_quad(mu_rect(x, y, src.w, src.h), src, color);
}


// Reliable?
// EFI_HII_FONT_GLYPH_GENERATOR_PROTOCOL
// EFI_HII_FONT_PACKAGE_HDR
// EFI_HII_GLYPH_INFO
// EFI_HII_GET_GLYPH_INFO
// EFI_HII_GENERATE_GLYPH_IMAGE

void Dada(void) {
  EFI_STATUS Status;
  Status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid,
    NULL, (VOID**) &pGO);
  gST->ConOut->OutputString(gST->ConOut,
    Status == EFI_SUCCESS ? u"OK\r\n" : u"KO\r\n");
  if (Status != EFI_SUCCESS) {
    return;
  }


  mu_init(&ctx);
  ctx.text_width = text_width;
  ctx.text_height = text_height;

  mu_begin(&ctx);
  if (mu_begin_window(&ctx, TR_L( "My Window" ), mu_rect(10, 10, 240, 86))) {
    // TODO:
    // https://deepwiki.com/rxi/microui/3.3-layout-system
    mu_layout_row(&ctx, 2, (int[]) { 100, -1 }, 0);

    mu_label(&ctx, TR_L( "First:" ));
    if (mu_button(&ctx, TR_L( "Button1" ))) {
      gST->ConOut->OutputString(gST->ConOut,
        TR_L( "Button1 pressed" ) TR_CRLF);
    }

    mu_label(&ctx, TR_L( "Second dada:" ));
    if (mu_button(&ctx, TR_L( "Button2" ))) {
      mu_open_popup(&ctx, TR_L( "My Popup" ));
    }

    if (mu_begin_popup(&ctx, TR_L( "My Popup" ))) {
      mu_label(&ctx, TR_L( "Hello world!" ));
      mu_end_popup(&ctx);
    }

    mu_end_window(&ctx);
  }
  mu_end(&ctx);


  mu_Command *cmd = NULL;
  while (mu_next_command(&ctx, &cmd)) {
    switch (cmd->type) {
      case MU_COMMAND_TEXT: {
        r_draw_text(cmd->text.str, cmd->text.pos, cmd->text.color);
      } break;

      case MU_COMMAND_RECT: {
        // r_draw_rect(cmd->rect.rect, cmd->rect.color);
        push_quad(cmd->rect.rect, cmd->rect.color);
      } break;

      case MU_COMMAND_ICON: {
        r_draw_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color);
        // push_quad(cmd->icon.rect, cmd->icon.color);
      } break;

      case MU_COMMAND_CLIP: {
        // r_set_clip_rect(cmd->clip.rect);
        clip = cmd->clip.rect;
      } break;
    }
  }


  draw_atlas_texture((mu_Vec2) { 0, 100 });
  draw_atlas((mu_Vec2) { 150, 100 });

  pGO->Blt(pGO, Screen, EfiBltBufferToVideo, 0, 0, 0, 0, WW, HH, 0);
  // Stall

}





















EFI_STATUS EFIAPI EfiMain(
  IN EFI_HANDLE TR_UNUSED ImageHandle,
  IN EFI_SYSTEM_TABLE* SystemTable)
{
  TrEfiInit(SystemTable);

  gST->ConOut->ClearScreen(gST->ConOut);
  Dada();
  gST->ConOut->OutputString(gST->ConOut,
    TR_L( "Hello World!" ) TR_CRLF);
  g_dadafafa[TR_ARRAYSIZE(g_dadafafa) - 1] = 0;
  gST->ConOut->OutputString(gST->ConOut, g_dadafafa);
  TR_LOG_LITERAL("Type \"Test\" and press <Enter>.");

  TrRunTests();

  EFI_INPUT_KEY Key;
  while (gST->ConIn->ReadKeyStroke(gST->ConIn, &Key) != EFI_SUCCESS);
  gRT->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);

  return EFI_SUCCESS;
}
