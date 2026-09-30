#include "Format.h"
#include "Unicode.h"
#include "Memory.h"
#include "Test.h"
#include "Math.h"

#define TR_FORMAT_FLAG_LEFT (1 << 0U)
#define TR_FORMAT_FLAG_PLUS (1 << 1U)
#define TR_FORMAT_FLAG_SPACE (1 << 2U)
#define TR_FORMAT_FLAG_ZEROPAD (1 << 3U)
#define TR_FORMAT_FLAG_ALTERNATE (1 << 4U)
#define TR_FORMAT_FLAG_PRECISION (1 << 5U)
#define TR_FORMAT_FLAG_UPPERCASE (1 << 6U)

// Should be large enough to contain any integer.
#define TR_FORMAT_FIXED_BUFFER_MAX_WIDTH 64U

typedef enum {
  TR_FORMAT_LENGTH_NONE = 0,
  TR_FORMAT_LENGTH_CHAR,
  TR_FORMAT_LENGTH_SHORT,
  TR_FORMAT_LENGTH_LONG,
  TR_FORMAT_LENGTH_LONG_LONG,
} TR_FORMAT_LENGTH;

typedef struct {
  UINTN Start, Index;
  CHAR16 Data[TR_FORMAT_FIXED_BUFFER_MAX_WIDTH];
} TR_FORMAT_FIXED_BUFFER;

typedef struct {
  UINTN Index;
  UINTN MaxWidth;
  CHAR16* Data;
} TR_FORMAT_VARIABLE_BUFFER;

typedef struct {
  UINTN Flags, Width, Precision;
  TR_FORMAT_LENGTH Length;
} TR_FORMAT_CONTEXT;

STATIC INLINE VOID TrFormatPutCharacter(
  IN TR_FORMAT_OUT_CALLBACK* Callback,
  IN CHAR16 Character)
{
  Callback->OutCharacter(Callback->Data, Character);
}

STATIC INLINE VOID TrFormatTerminate(
  IN TR_FORMAT_OUT_CALLBACK* Callback)
{
  Callback->Terminate(Callback->Data);
}

STATIC INLINE BOOLEAN TrFormatIsDigit(
  IN CHAR16 Character)
{
  return Character >= TR_L( '0' ) && Character <= TR_L( '9' );
}

STATIC UINTN TrFormatParseUintn(
  IN OUT CHAR16 CONST** Format)
{
  UINTN Value = 0U;
  while (TrFormatIsDigit(**Format)) {
    UINTN Unit = **Format - TR_L( '0' );
    Value = Value * 10U + Unit;
    ++(*Format);
  }
  return Value;
}

STATIC VOID TrFormatParseFlags(
  IN OUT CHAR16 CONST** Format,
  IN OUT TR_FORMAT_CONTEXT* Context)
{
  while (1) {
    switch (**Format) {
      case TR_L( '-' ):
        Context->Flags |= TR_FORMAT_FLAG_LEFT;
        ++(*Format);
        continue;

      case TR_L( '+' ):
        Context->Flags |= TR_FORMAT_FLAG_PLUS;
        ++(*Format);
        continue;

      case TR_L( ' ' ):
        Context->Flags |= TR_FORMAT_FLAG_SPACE;
        ++(*Format);
        continue;

      case TR_L( '0' ):
        Context->Flags |= TR_FORMAT_FLAG_ZEROPAD;
        ++(*Format);
        continue;

      case TR_L( '#' ):
        Context->Flags |= TR_FORMAT_FLAG_ALTERNATE;
        ++(*Format);
        continue;

      default:
        return;
    }
  }
}

STATIC VOID TrFormatParseWidth(
  IN OUT CHAR16 CONST** Format,
  IN OUT TR_FORMAT_CONTEXT* Context,
  IN OUT VA_LIST* Args)
{
  if (**Format != TR_L( '*' )) {
    // TODO: ParseInt() and if < 0 then FLAG_LEFT ?
    Context->Width = TrFormatParseUintn(Format);
    return;
  }

  ++(*Format);
  INT GivenWidth = VA_ARG(*Args, INT);
  Context->Width = (UINTN) TR_ABS(GivenWidth);
  if (GivenWidth < 0) {
    Context->Flags |= TR_FORMAT_FLAG_LEFT;
  }
}

STATIC VOID TrFormatParsePrecision(
  IN OUT CHAR16 CONST** Format,
  IN OUT TR_FORMAT_CONTEXT* Context,
  IN OUT VA_LIST* Args)
{
  if (**Format != TR_L( '.' )) {
    return;
  }

  ++(*Format);
  Context->Flags |= TR_FORMAT_FLAG_PRECISION;
  if (**Format != TR_L( '*' )) {
    Context->Precision = TrFormatParseUintn(Format);
    return;
  }

  INT GivenPrecision = VA_ARG(*Args, INT);
  Context->Precision = (UINTN) TR_MAX(0, GivenPrecision);
  ++(*Format);
}

STATIC VOID TrFormatParseLength(
  IN OUT CHAR16 CONST** Format,
  IN OUT TR_FORMAT_CONTEXT* Context)
{
  switch (**Format) {
    case TR_L( 'l' ):
      ++(*Format);
      Context->Length = **Format == TR_L( 'l' )
        ? (++(*Format), TR_FORMAT_LENGTH_LONG_LONG)
        : TR_FORMAT_LENGTH_LONG;
      break;

    case TR_L( 'h' ):
      ++(*Format);
      Context->Length = **Format == TR_L( 'h' )
        ? (++(*Format), TR_FORMAT_LENGTH_CHAR)
        : TR_FORMAT_LENGTH_SHORT;
      break;

    case TR_L( 't' ):
      ++(*Format);
      #ifdef TR_FORMAT_SUPPORT_PTRDIFF
        Context->Length = sizeof(PTRDIFF_T) == sizeof(LONG)
          ? TR_FORMAT_LENGTH_LONG : TR_FORMAT_LENGTH_LONG_LONG;
      #endif
      break;

    case TR_L( 'j' ):
      ++(*Format);
      #ifdef TR_FORMAT_SUPPORT_INTMAX
        Context->Length = sizeof(INTMAX_T) == sizeof(LONG)
          ? TR_FORMAT_LENGTH_LONG : TR_FORMAT_LENGTH_LONG_LONG;
      #endif
      break;

    case TR_L( 'z' ):
      ++(*Format);
      Context->Length = sizeof(SIZE_T) == sizeof(LONG)
        ? TR_FORMAT_LENGTH_LONG : TR_FORMAT_LENGTH_LONG_LONG;
      break;

    default:
      break;
  }
}

STATIC VOID TrFormatConvertChar(
  IN TR_FORMAT_OUT_CALLBACK* Callback,
  IN OUT TR_FORMAT_CONTEXT CONST* Context,
  IN VA_LIST* Args)
{
  // TODO: %lc, %c
  CHAR16 Character = (CHAR16) VA_ARG(*Args, INT);
  (void) Callback;
  (void) Context;
  (void) Character;
}

STATIC VOID TrFormatConvertString8(
  IN TR_FORMAT_OUT_CALLBACK* Callback,
  IN OUT TR_FORMAT_CONTEXT CONST* Context,
  IN CHAR8 CONST* String)
{
  (void) Callback;
  (void) Context;
  (void) String;

  // TODO: Width & Precision
  while (*String != TR_L( '\0' )) {
    UINT32 CodePoint = 0U;
    UINTN SequenceLength = 0U;
    // TODO: 4U = Utf8StringLength = Out of bounds.
    if (TrUtf8ToCodePoint(String, 4U, &SequenceLength, &CodePoint)) {
      String += SequenceLength;
      CHAR16 Utf16Bytes[2] = { 0x0 };
      SequenceLength = TrCodePointToUtf16(CodePoint, Utf16Bytes);
      if (SequenceLength >= 1U) {
        TrFormatPutCharacter(Callback, Utf16Bytes[0]);
        if (SequenceLength >= 2U) {
          TrFormatPutCharacter(Callback, Utf16Bytes[1]);
        }
      }
    } else {
      ++String;
    }
  }
}

STATIC VOID TrFormatConvertString16(
  IN TR_FORMAT_OUT_CALLBACK* Callback,
  IN OUT TR_FORMAT_CONTEXT CONST* Context,
  IN CHAR16 CONST* String)
{
  // TODO: Width & Precision
  UINTN Index = 0U;
  UINTN Width = TR_VALUE_OR(Context->Width, (UINTN) -1);
  while (*String != TR_L( '\0' ) && Index < Width) {
    TrFormatPutCharacter(Callback, *String);
    ++String; ++Index;
  }
}

STATIC VOID TrFormatConvertString(
  IN TR_FORMAT_OUT_CALLBACK* Callback,
  IN OUT TR_FORMAT_CONTEXT CONST* Context,
  IN VA_LIST* Args)
{
  TR_FORMAT_LENGTH Length = Context->Length;
  VOID CONST* String = VA_ARG(*Args, VOID CONST*);
  if (String == NULL) {
    String = TR_L( "(null)" );
    Length = TR_FORMAT_LENGTH_NONE;
  }

  if (Context->Length == TR_FORMAT_LENGTH_SHORT) {
    TrFormatConvertString8(Callback, Context,
      (CHAR8 CONST*) String);
  } else {
    TrFormatConvertString16(Callback, Context,
      (CHAR16 CONST*) String);
  }
}

STATIC VOID TrFormatWriteIntegerBuffer(
  IN TR_FORMAT_OUT_CALLBACK* Callback,
  IN OUT TR_FORMAT_CONTEXT CONST* Context,
  IN TR_FORMAT_FIXED_BUFFER CONST* Integer,
  IN CHAR16 Format,
  IN BOOLEAN IsNegative)
{
  UINTN MinimumWidth = Integer->Index;

  // Minimum number of digits (padded with zéro).
  if (TR_HAS_FLAGS(Context->Flags, TR_FORMAT_FLAG_PRECISION)) {
    MinimumWidth = TR_MAX(MinimumWidth, Context->Precision);
  }

  // Prefixed with "0x", "0b", "0o", etc.
  if (TR_HAS_FLAGS(Context->Flags, TR_FORMAT_FLAG_ALTERNATE)) {
    MinimumWidth += 2;
  }

  // Prefixed with a "-", "+" or a space.
  UINTN CONST SignFlags = TR_FORMAT_FLAG_PLUS | TR_FORMAT_FLAG_SPACE;
  if (IsNegative || (Context->Flags & SignFlags)) {
    MinimumWidth += 1;
  }

  UINTN ActualPad = TR_SATURATING_SUB(Context->Width, MinimumWidth);

  // Padding right (SPACE):
  // ┌──────────┬──────┬────────┬───────────┬─────────┐
  // │ SpacePad │ Sign │ Prefix │ Precision │ Integer │
  // └──────────┴──────┴────────┴───────────┴─────────┘
  if (!TR_HAS_FLAGS(Context->Flags, TR_FORMAT_FLAG_LEFT)) {
    if (!TR_HAS_FLAGS(Context->Flags, TR_FORMAT_FLAG_ZEROPAD)) {
      for (UINTN Index = 0U; Index < ActualPad; ++Index) {
        TrFormatPutCharacter(Callback, TR_L( ' ' ));
      }
    }
  }

  if (IsNegative) {
    TrFormatPutCharacter(Callback, TR_L( '-' ));
  } else if (Context->Flags & SignFlags) {
    TrFormatPutCharacter(Callback,
      Context->Flags & TR_FORMAT_FLAG_PLUS
        ? TR_L( '+' ) : TR_L( ' ' ));
  }

  if (TR_HAS_FLAGS(Context->Flags, TR_FORMAT_FLAG_ALTERNATE)) {
    TrFormatPutCharacter(Callback, TR_L( '0' ));
    // NOTE (me): I prefer lowercase letter here.
    TrFormatPutCharacter(Callback, (Format | 0x20));
  }

  // Padding right (ZERO):
  // ┌──────┬────────┬─────────┬───────────┬─────────┐
  // │ Sign │ Prefix │ ZeroPad │ Precision │ Integer │
  // └──────┴────────┴─────────┴───────────┴─────────┘
  if (!TR_HAS_FLAGS(Context->Flags, TR_FORMAT_FLAG_LEFT)) {
    if (TR_HAS_FLAGS(Context->Flags, TR_FORMAT_FLAG_ZEROPAD)) {
      for (UINTN Index = 0U; Index < ActualPad; ++Index) {
        TrFormatPutCharacter(Callback, TR_L( '0' ));
      }
    }
  }

  if (TR_HAS_FLAGS(Context->Flags, TR_FORMAT_FLAG_PRECISION)) {
    UINTN ActualPrecision = TR_SATURATING_SUB(
      Context->Precision, Integer->Index);
    for (UINTN Index = 0U; Index < ActualPrecision; ++Index) {
      TrFormatPutCharacter(Callback, TR_L( '0' ));
    }
  }

  for (UINTN Index = Integer->Index; Index > 0U; --Index) {
    TrFormatPutCharacter(Callback, Integer->Data[Index - 1U]);
  }

  // Padding left (SPACE only):
  // ┌──────┬────────┬───────────┬─────────┬──────────┐
  // │ Sign │ Prefix │ Precision │ Integer │ SpacePad │
  // └──────┴────────┴───────────┴─────────┴──────────┘
  if (TR_HAS_FLAGS(Context->Flags, TR_FORMAT_FLAG_LEFT)) {
    for (UINTN Index = 0U; Index < ActualPad; ++Index) {
      TrFormatPutCharacter(Callback, TR_L( ' ' ));
    }
  }
}

STATIC VOID TrFormatConvertIntegerBuffer(
  IN OUT TR_FORMAT_FIXED_BUFFER* Buffer,
  IN TR_FORMAT_CONTEXT CONST* Context,
  IN ULONG_LONG Base,
  IN ULONG_LONG Value,
  IN BOOLEAN StripTrailingZero)
{
  BOOLEAN HasNonZeroDigit = FALSE;
  BOOLEAN IsUppercased = TR_HAS_FLAGS(Context->Flags,
    TR_FORMAT_FLAG_UPPERCASE);

  do {
    ULONG_LONG Remainder = Value % Base; Value /= Base;
    if (Buffer->Index < TR_FORMAT_FIXED_BUFFER_MAX_WIDTH) {
      if (StripTrailingZero && !HasNonZeroDigit && Remainder != 0U) {
        Buffer->Start = Buffer->Index;
        HasNonZeroDigit = TRUE;
      }

      Buffer->Data[Buffer->Index++] = (Remainder < 10)
        ? (CHAR16) Remainder + TR_L( '0' )
        : (CHAR16) (Remainder - 10U) + (IsUppercased
          ? TR_L( 'A' ) : TR_L( 'a' ));
    } else break;
  } while (Value > 0);

  if (StripTrailingZero && !HasNonZeroDigit) {
    Buffer->Start = Buffer->Index;
  }
}

STATIC VOID TrFormatConvertIntegerBase(
  IN TR_FORMAT_OUT_CALLBACK* Callback,
  IN OUT TR_FORMAT_CONTEXT CONST* Context,
  IN CHAR16 Format,
  IN ULONG_LONG Base,
  IN ULONG_LONG Value,
  IN BOOLEAN IsNegative)
{
  TR_FORMAT_FIXED_BUFFER TemporaryBuffer = { 0x0 };
  TrFormatConvertIntegerBuffer(&TemporaryBuffer, Context,
    Base, Value, /*StripTrailingZero*/ FALSE);

  TrFormatWriteIntegerBuffer(Callback, Context,
    &TemporaryBuffer, Format, IsNegative);
}

// ⚠️ NOTE: This macro contains side-effects.
// This macro is intended to be used by `TrFormatConvertIntegerArgs()`.
#define TR_READ_ARGS(SIGNED_TYPE, UNSIGNED_TYPE)             \
  if (!IsSigned) {                                           \
    UNSIGNED_TYPE GivenValue = VA_ARG(*Args, UNSIGNED_TYPE); \
    Value = (ULONG_LONG) GivenValue;                         \
  } else {                                                   \
    SIGNED_TYPE GivenValue = VA_ARG(*Args, SIGNED_TYPE);     \
    Value = (ULONG_LONG) ((IsNegative = GivenValue < 0)      \
      ? (0 - GivenValue) : GivenValue);                      \
  }

STATIC VOID TrFormatConvertIntegerArgs(
  IN TR_FORMAT_OUT_CALLBACK* Callback,
  IN OUT TR_FORMAT_CONTEXT* Context,
  IN CHAR16 Format,
  IN ULONG_LONG Base,
  IN BOOLEAN IsSigned,
  IN VA_LIST* Args)
{
  ULONG_LONG Value = 0U;
  BOOLEAN IsNegative = FALSE;

  switch (Context->Length) {
    case TR_FORMAT_LENGTH_LONG_LONG: {
      TR_READ_ARGS(LONG_LONG, ULONG_LONG);
      break;
    }

    case TR_FORMAT_LENGTH_LONG: {
      TR_READ_ARGS(LONG, ULONG);
      break;
    }

    case TR_FORMAT_LENGTH_NONE:
    case TR_FORMAT_LENGTH_SHORT:
    case TR_FORMAT_LENGTH_CHAR:
    default: {
      TR_READ_ARGS(INT, UINTN);
      break;
    }
  }

  TrFormatConvertIntegerBase(Callback, Context,
    Format, Base, Value, IsNegative);
}

STATIC VOID TrFormatConvertInteger(
  IN TR_FORMAT_OUT_CALLBACK* Callback,
  IN OUT TR_FORMAT_CONTEXT* Context,
  IN CHAR16 Format,
  IN VA_LIST* Args)
{
  ULONG_LONG Base = 10U;
  BOOLEAN IsSigned = FALSE;

  switch (Format) {
    case TR_L( 'i' ):
    case TR_L( 'd' ):
      IsSigned = TRUE;
      break;

    case TR_L( 'b' ):
      Base = 2U;
      break;

    case TR_L( 'o' ):
      Base = 8U;
      break;

    case TR_L( 'x' ):
      Base = 16U;
      break;

    case TR_L( 'X' ):
      Context->Flags |= TR_FORMAT_FLAG_UPPERCASE;
      Base = 16U;
      break;
  }

  TrFormatConvertIntegerArgs(Callback, Context,
    Format, Base, IsSigned, Args);
}

STATIC VOID TrFormatConvertDoubleDecimal(
  IN TR_FORMAT_OUT_CALLBACK* Callback,
  IN OUT TR_FORMAT_CONTEXT CONST* Context,
  IN BOOLEAN IsNegative,
  IN DOUBLE GivenValue)
{
  (void) Callback;
  (void) Context;
  (void) IsNegative;
  (void) GivenValue;
}

STATIC VOID TrFormatConvertDoubleScientific(
  IN TR_FORMAT_OUT_CALLBACK* Callback,
  IN OUT TR_FORMAT_CONTEXT CONST* Context,
  IN BOOLEAN IsNegative,
  IN DOUBLE GivenValue)
{
  (void) Callback;
  (void) Context;
  (void) IsNegative;
  (void) GivenValue;

  // TODO TMP WIP
  // TODO: TrPow10Double()
  DOUBLE Exponent = TrLog10Double(GivenValue);
  DOUBLE Decimals = GivenValue / TrPow10Double(Exponent);
  // printf("%f.e%llu\n", Decimals, Exponent);
  (void) Exponent;
  (void) Decimals;

  // Default precision = 6
  // Exponent always two digits
  // [-]d.ddde±dd
}

STATIC VOID TrFormatConvertDoubleAdaptive(
  IN TR_FORMAT_OUT_CALLBACK* Callback,
  IN OUT TR_FORMAT_CONTEXT CONST* Context,
  IN BOOLEAN IsNegative,
  IN DOUBLE GivenValue)
{
  (void) Callback;
  (void) Context;
  (void) IsNegative;
  (void) GivenValue;
}

STATIC VOID TrFormatWriteDoubleHexadecimal(
  IN TR_FORMAT_OUT_CALLBACK* Callback,
  IN OUT TR_FORMAT_CONTEXT CONST* Context,
  IN CHAR16 InvisibleDigit,
  IN TR_FORMAT_FIXED_BUFFER CONST* Mantissa,
  IN TR_FORMAT_FIXED_BUFFER CONST* Exponent,
  IN BOOLEAN IsExponentNegative,
  IN BOOLEAN IsNegative)
{
  // ┌──────┬─────────┬───┬──────────┬───────┬──────────┐
  // │ Sign │ 0x[012] │ . │ Mantissa │ p[+-] │ Exponent │
  // └──────┴─────────┴───┴──────────┴───────┴──────────┘
  // Always contains "0x[012]" and "p[+-]".
  UINTN MinimumWidth = 5 + Exponent->Index;
  UINTN MantissaSize = TR_SATURATING_SUB(Mantissa->Index, Mantissa->Start);
  if (TR_HAS_FLAGS(Context->Flags, TR_FORMAT_FLAG_PRECISION)) {
    MantissaSize = Context->Precision;
  }

  // Minimum number of mantissa digits (padded with zéro).
  MinimumWidth += MantissaSize;

  // Prefixed with a "-", "+" or a space.
  UINTN CONST SignFlags = TR_FORMAT_FLAG_PLUS | TR_FORMAT_FLAG_SPACE;
  if (IsNegative || (Context->Flags & SignFlags)) {
    MinimumWidth += 1;
  }

  // Separate with the decimal point ".".
  if (TR_HAS_FLAGS(Context->Flags, TR_FORMAT_FLAG_ALTERNATE)
      || (TR_HAS_FLAGS(Context->Flags, TR_FORMAT_FLAG_PRECISION)
        && Context->Precision > 0U)
      || (!TR_HAS_FLAGS(Context->Flags, TR_FORMAT_FLAG_PRECISION)
        && Mantissa->Index > Mantissa->Start)) {
    MinimumWidth += 1;
  }

  UINTN ActualPad = TR_SATURATING_SUB(Context->Width, MinimumWidth);

  // Padding right (SPACE):
  // ┌──────────┬──────┬─────────┬───┬──────────┬───────┬──────────┐
  // │ SpacePad │ Sign │ 0x[012] │ . │ Mantissa │ p[+-] │ Exponent │
  // └──────────┴──────┴─────────┴───┴──────────┴───────┴──────────┘
  if (!TR_HAS_FLAGS(Context->Flags, TR_FORMAT_FLAG_LEFT)) {
    if (!TR_HAS_FLAGS(Context->Flags, TR_FORMAT_FLAG_ZEROPAD)) {
      for (UINTN Index = 0U; Index < ActualPad; ++Index) {
        TrFormatPutCharacter(Callback, TR_L( ' ' ));
      }
    }
  }

  if (IsNegative) {
    TrFormatPutCharacter(Callback, TR_L( '-' ));
  } else if (Context->Flags & SignFlags) {
    TrFormatPutCharacter(Callback,
      Context->Flags & TR_FORMAT_FLAG_PLUS
        ? TR_L( '+' ) : TR_L( ' ' )
    );
  }

  // NOTE (me): I prefer lowercase letter here.
  TrFormatPutCharacter(Callback, TR_L( '0' ));
  TrFormatPutCharacter(Callback, TR_L( 'x' ));

  // Padding right (ZERO):
  // ┌──────┬────┬─────────┬───────┬───┬──────────┬───────┬──────────┐
  // │ Sign │ 0x │ ZeroPad │ [012] │ . │ Mantissa │ p[+-] │ Exponent │
  // └──────┴────┴─────────┴───────┴───┴──────────┴───────┴──────────┘
  if (!TR_HAS_FLAGS(Context->Flags, TR_FORMAT_FLAG_LEFT)) {
    if (TR_HAS_FLAGS(Context->Flags, TR_FORMAT_FLAG_ZEROPAD)) {
      for (UINTN Index = 0U; Index < ActualPad; ++Index) {
        TrFormatPutCharacter(Callback, TR_L( '0' ));
      }
    }
  }

  TrFormatPutCharacter(Callback, InvisibleDigit);

  if (TR_HAS_FLAGS(Context->Flags, TR_FORMAT_FLAG_ALTERNATE)
      || (TR_HAS_FLAGS(Context->Flags, TR_FORMAT_FLAG_PRECISION)
        && Context->Precision > 0U)
      || (!TR_HAS_FLAGS(Context->Flags, TR_FORMAT_FLAG_PRECISION)
        && Mantissa->Index > Mantissa->Start)) {
    TrFormatPutCharacter(Callback, TR_L( '.' ));
  }

  UINTN MantissaCount = 0U;
  for (UINTN Index = Mantissa->Index; Index > Mantissa->Start
      && MantissaCount < MantissaSize; --Index, ++MantissaCount) {
    TrFormatPutCharacter(Callback, Mantissa->Data[Index - 1U]);
  }

  // TODO: Should mantissa be rounded?
  for (; MantissaCount < MantissaSize; ++MantissaCount) {
    TrFormatPutCharacter(Callback, TR_L( '0' ));
  }

  TrFormatPutCharacter(Callback, TR_L( 'p' ));
  TrFormatPutCharacter(Callback, IsExponentNegative ? TR_L( '-' ) : TR_L( '+' ));
  for (UINTN Index = Exponent->Index; Index > 0U; --Index) {
    TrFormatPutCharacter(Callback, Exponent->Data[Index - 1U]);
  }

  // Padding left (SPACE only):
  // ┌──────┬─────────┬───┬──────────┬───────┬──────────┬──────────┐
  // │ Sign │ 0x[012] │ . │ Mantissa │ p[+-] │ Exponent │ SpacePad │
  // └──────┴─────────┴───┴──────────┴───────┴──────────┴──────────┘
  if (TR_HAS_FLAGS(Context->Flags, TR_FORMAT_FLAG_LEFT)) {
    for (UINTN Index = 0U; Index < ActualPad; ++Index) {
      TrFormatPutCharacter(Callback, TR_L( ' ' ));
    }
  }
}

STATIC VOID TrFormatConvertDoubleHexadecimal(
  IN TR_FORMAT_OUT_CALLBACK* Callback,
  IN OUT TR_FORMAT_CONTEXT CONST* Context,
  IN BOOLEAN IsNegative,
  IN DOUBLE GivenValue)
{
  TR_STATIC_ASSERT(sizeof(DOUBLE) == sizeof(UINT64));
  union { DOUBLE D; UINT64 U; } Value = { GivenValue };

  ULONG_LONG Mantissa = Value.U & TR_DOUBLE_MANTISSA_MASK;
  LONG_LONG Exponent = (LONG_LONG) ((Value.U & TR_DOUBLE_EXPONENT_MASK) >> 52U) - 1023LL;
  BOOLEAN IsExponentZeroed = (Value.U & TR_DOUBLE_EXPONENT_MASK) == 0U;
  BOOLEAN IsExponentNegative = !IsExponentZeroed && Exponent < 0;

  CHAR16 InvisibleDigit = TR_L( '1' );
  if (IsExponentZeroed) {
    InvisibleDigit = TR_L( '0' );
  } else if (TR_HAS_FLAGS(Context->Flags, TR_FORMAT_FLAG_PRECISION)
      && Context->Precision == 0U) {
    Value.U = (Value.U & TR_DOUBLE_MANTISSA_MASK) | (1023ULL << 52U); // [1, 2)
    InvisibleDigit = TR_L( '0' ) + (CHAR16) (Value.D + 0.5);
  }

  TR_FORMAT_FIXED_BUFFER MantissaBuffer = { 0x0 };
  TrFormatConvertIntegerBuffer(&MantissaBuffer, Context,
    /*Base*/ 16U, Mantissa, /*StripTrailingZero*/ TRUE);

  TR_FORMAT_FIXED_BUFFER ExponentBuffer = { 0x0 };
  TrFormatConvertIntegerBuffer(&ExponentBuffer, Context,
    /*Base*/ 10U, /*Value*/ (ULONG_LONG) (IsExponentZeroed
      ? 0 : (IsExponentNegative ? 0 - Exponent : Exponent)),
    /*StripTrailingZero*/ FALSE);

  TrFormatWriteDoubleHexadecimal(Callback, Context,
    InvisibleDigit, &MantissaBuffer, &ExponentBuffer,
    IsExponentNegative, IsNegative);
}

STATIC VOID TrFormatConvertDouble(
  IN TR_FORMAT_OUT_CALLBACK* Callback,
  IN OUT TR_FORMAT_CONTEXT* Context,
  IN CHAR16 Format,
  IN VA_LIST* Args)
{
  // TODO: Handle denormalized values.
  if (Context->Length == TR_FORMAT_LENGTH_LONG_LONG) {
    LONG_DOUBLE GivenValue = VA_ARG(*Args, LONG_DOUBLE);
    (void) GivenValue;
    return;
  }

  DOUBLE GivenValue = VA_ARG(*Args, DOUBLE);
  BOOLEAN IsNegative = GivenValue < 0;
  GivenValue = TR_ABS(GivenValue);

  // TODO
  // if (value != value)   // NaN
  // if (value < -DBL_MAX) // -inf
  // if (value > DBL_MAX)  // +inf ou inf

  if ((Format & 0x20) == 0U) {
    Context->Flags |= TR_FORMAT_FLAG_UPPERCASE;
  }

  switch (Format | 0x20) {
    case TR_L( 'f' ):
      TrFormatConvertDoubleDecimal(
        Callback, Context, IsNegative, GivenValue);

    case TR_L( 'e' ):
      TrFormatConvertDoubleScientific(
        Callback, Context, IsNegative, GivenValue);
      break;

    case TR_L( 'g' ):
      TrFormatConvertDoubleAdaptive(
        Callback, Context, IsNegative, GivenValue);
      break;

    case TR_L( 'a' ):
      TrFormatConvertDoubleHexadecimal(
        Callback, Context, IsNegative, GivenValue);
      break;
  }
}

STATIC VOID TrFormatConvert(
  IN TR_FORMAT_OUT_CALLBACK* Callback,
  IN OUT TR_FORMAT_CONTEXT* Context,
  IN OUT CHAR16 CONST** Format,
  IN VA_LIST* Args)
{
  // TODO: %C = %lc and %S = %ls
  switch (**Format) {
    case TR_L( 'c' ):
      TrFormatConvertChar(Callback, Context, Args);
      ++(*Format);
      break;

    case TR_L( 's' ):
      TrFormatConvertString(Callback, Context, Args);
      ++(*Format);
      break;

    case TR_L( 'b' ):
    case TR_L( 'o' ):
    case TR_L( 'x' ):
    case TR_L( 'X' ):
    case TR_L( 'd' ):
    case TR_L( 'i' ):
    case TR_L( 'u' ):
      TrFormatConvertInteger(Callback, Context, **Format, Args);
      ++(*Format);
      break;

    case TR_L( 'p' ):
      (void) TrFormatConvertDouble;
      ++(*Format);
      break;

    case TR_L( 'f' ):
    case TR_L( 'F' ):
    case TR_L( 'e' ):
    case TR_L( 'E' ):
    case TR_L( 'g' ):
    case TR_L( 'G' ):
    case TR_L( 'a' ):
    case TR_L( 'A' ):
      TrFormatConvertDouble(Callback, Context, **Format, Args);
      ++(*Format);
      break;

    default:
      TrFormatPutCharacter(Callback, **Format);
      ++(*Format);
      break;
  }
}

VOID TrFormatOutV(
  IN TR_FORMAT_OUT_CALLBACK* Callback,
  IN CHAR16 CONST* Format,
  IN VA_LIST Args)
{
  while (*Format != TR_L( '\0' )) {
    if (*Format != TR_L( '%' )) {
      TrFormatPutCharacter(Callback, *(Format++));
      continue;
    }

    ++Format; // Skip '%'.
    TR_FORMAT_CONTEXT Context = {
      .Flags = 0U, .Width = 0U, .Precision = 0U,
      .Length = TR_FORMAT_LENGTH_NONE,
    };

    TrFormatParseFlags(&Format, &Context);
    TrFormatParseWidth(&Format, &Context, &Args);
    TrFormatParsePrecision(&Format, &Context, &Args);
    TrFormatParseLength(&Format, &Context);
    TrFormatConvert(Callback, &Context, &Format, &Args);
  }

  TrFormatTerminate(Callback);
}

UINTN TrFormat(
  OUT CHAR16* Buffer OPTIONAL,
  IN UINTN N,
  IN CHAR16 CONST* Format,
  IN ...)
{
  VA_LIST Args;
  VA_START(Args, Format);
  UINTN result = TrFormatV(Buffer, N, Format, Args);
  VA_END(Args);
  return result;
}

STATIC VOID TrFormatVariableBufferOutCharacter(
  IN VOID* Data,
  IN CHAR16 Character)
{
  TR_FORMAT_VARIABLE_BUFFER* Buffer = (TR_FORMAT_VARIABLE_BUFFER*) Data;
  if (Buffer->Data != NULL) {
    if (Buffer->Index < Buffer->MaxWidth) {
      Buffer->Data[Buffer->Index] = Character;
    }
  }
  Buffer->Index++;
}

STATIC VOID TrFormatVariableBufferTerminate(
  IN VOID* Data)
{
  TR_FORMAT_VARIABLE_BUFFER* Buffer = (TR_FORMAT_VARIABLE_BUFFER*) Data;
  if (Buffer->Data != NULL) {
    UINTN Index = Buffer->Index < Buffer->MaxWidth
      ? Buffer->Index : Buffer->MaxWidth - 1U;
    Buffer->Data[Index] = TR_L( '\0' );
  }
  Buffer->Index++;
}

UINTN TrFormatV(
  OUT CHAR16* Buffer OPTIONAL,
  IN UINTN N,
  IN CHAR16 CONST* Format,
  IN VA_LIST Args)
{
  TR_FORMAT_VARIABLE_BUFFER VariableBuffer = {
    .Index = 0U, .MaxWidth = N, .Data = Buffer
  };

  TR_FORMAT_OUT_CALLBACK Callback = {
    .Data = (VOID*) &VariableBuffer,
    .Terminate = TrFormatVariableBufferTerminate,
    .OutCharacter = TrFormatVariableBufferOutCharacter,
  };

  TrFormatOutV(&Callback, Format, Args);
  return VariableBuffer.Index;
}

#define TR_FORMAT_ASSERT(EXPECTED, FORMAT, ...) do {                      \
    CHAR16 Buffer[128] = { 0x0 };                                         \
    UINTN ExpectedSize = SIZEOF(TR_L(EXPECTED)) / SIZEOF(CHAR16);         \
    UINTN Written = TrFormat(Buffer, 128, TR_L(FORMAT), __VA_ARGS__);     \
    if (Written != ExpectedSize || 0 != TrStringCompare(                  \
        Buffer, TR_L(EXPECTED), TR_MIN(Written, 128))) {                  \
      TR_LPRINTLN("  %hs: \"%*ls\"(%d) != \"%ls\"(%d)", __func__,         \
        TR_MIN(Written, 128), Buffer, Written,                            \
        TR_L(EXPECTED), ExpectedSize);                                    \
      TR_TEST_FAILED();                                                   \
    }                                                                     \
  } while(0)

#include "Print.h"

TR_TEST(TrTestFormatBinary) {
  TR_FORMAT_ASSERT(  "10101001", "%b",      0xA9);
  TR_FORMAT_ASSERT("0b10101011", "%#b",     0xAB);
  TR_FORMAT_ASSERT("0b10101010", "%#10b",   0xAA);
  TR_FORMAT_ASSERT("  10101001", "%10b",    0xA9);
  TR_FORMAT_ASSERT("0010101001", "%.10b",   0xA9);
  TR_FORMAT_ASSERT("0010101001", "%10.10b", 0xA9);
  TR_FORMAT_ASSERT("000000011000000", "%.*b", 15, 0xC0);
  TR_FORMAT_ASSERT("       11000000", "%*.b", 15, 0xC0);
}

TR_TEST(TrTestFormatHexadecimal) {
  TR_FORMAT_ASSERT("00012b|",  "%06x|", 0x12B);
  TR_FORMAT_ASSERT("0x012C|", "%#06X|", 0x12C);
  TR_FORMAT_ASSERT(" 0x12d|",  "%#6x|", 0x12D);
  TR_FORMAT_ASSERT("00012E|",  "%.6X|", 0x12E);
}

TR_TEST(TrTestFormatDecimal) {
  TR_FORMAT_ASSERT( "123",   "%i",   123);
  TR_FORMAT_ASSERT( "123",   "%.i",  123);
  TR_FORMAT_ASSERT("-123",   "%i",  -123);
  TR_FORMAT_ASSERT("-123",   "%-i", -123);
  TR_FORMAT_ASSERT("-123",   "%+i", -123);
  TR_FORMAT_ASSERT("-123",   "% i", -123);
  TR_FORMAT_ASSERT(" 123",   "% i",  123);
  TR_FORMAT_ASSERT("+00123", "%+6.5i",  123);
  TR_FORMAT_ASSERT("-00123", "%+6.5i", -123);
  TR_FORMAT_ASSERT(" 00123",  "%6.5i",  123);
}

TR_TEST(TrTestFormatDoubleHexadecimal) {
  TR_FORMAT_ASSERT(                                                              "0x1.9p-1|",      "%0a|", 0.78125);
  TR_FORMAT_ASSERT(                                                          "0x1.90000p-1|",    "%0.5a|", 0.78125);
  TR_FORMAT_ASSERT(                  "0x1.900000000000000000000000000000000000000000000p-1|",   "%0.45a|", 0.78125);
  TR_FORMAT_ASSERT("                  0x1.900000000000000000000000000000000000000000000p-1|",  "%70.45a|", 0.78125);
  TR_FORMAT_ASSERT("0x1.900000000000000000000000000000000000000000000p-1                  |", "%-70.45a|", 0.78125);
  TR_FORMAT_ASSERT("0x0000000000000000001.900000000000000000000000000000000000000000000p-1|", "%070.45a|", 0.78125);

  TR_FORMAT_ASSERT(             "-0x2p+1|",    "%.0a|", -3.14159);
  TR_FORMAT_ASSERT("        -0x1.921fp+1|", "% 20.4a|", -3.14159);
  TR_FORMAT_ASSERT("-0x1.921Fp+1        |", "%-20.4A|", -3.14159);
  TR_FORMAT_ASSERT("-0x000000001.921Fp+1|", "%020.4A|", -3.14159);

  TR_FORMAT_ASSERT("-0x1.921f9f01b866ep+1",   "%+a", -3.14159);
  TR_FORMAT_ASSERT("+0x1.921f9f01b866ep+1",   "%+a",  3.14159);
  TR_FORMAT_ASSERT(" 0x1.921f9f01b866ep+1",   "% a",  3.14159);
  TR_FORMAT_ASSERT(              "-0x2p+1", "%+.0a", -3.14159);

  TR_FORMAT_ASSERT( "0x1.921f9f01b866ep+1", "%a",  3.14159);
  TR_FORMAT_ASSERT("-0x1.921f9f01b866ep+1", "%a", -3.14159);
  TR_FORMAT_ASSERT( "0x1.21f9f01b866e4p-3", "%a",  0.14159);
  TR_FORMAT_ASSERT( "0x1.E243F3E0370CEp+4", "%A", 30.14159);

  TR_FORMAT_ASSERT(   "0x0p+0|",    "%a|", 0.0);
  TR_FORMAT_ASSERT(  "0x0.p+0|",   "%#a|", 0.0);
  TR_FORMAT_ASSERT(   "0x1p-1|",  "%.0a|", 0.654);
  TR_FORMAT_ASSERT(  "0x1.p-1|", "%#.0a|", 0.654);
  TR_FORMAT_ASSERT(   "0x1p-1|",  "%.0a|", 0.654);
  TR_FORMAT_ASSERT("-0x1.8p+1|",   "%+a|", -3.0);
  TR_FORMAT_ASSERT("-0x1.9p-4|", "%+.1a|", -0.1);

  // TR_FORMAT_ASSERT(  " 0x1.0e7ad82221eecp-33",   "%22a", 0.000000000123);
  // TR_FORMAT_ASSERT(  "0x1.0e7ad82221eecp-33 ",   "%-22a", 0.000000000123);
  // TR_FORMAT_ASSERT(    "0x1.0e7bp-33         ", "%-22.4a", 0.000000000123);
  // TR_FORMAT_ASSERT(            "0x1.0e7aep-33", "%-22.5a", 0.000000000123);
  // TR_FORMAT_ASSERT(           "0x1.0e7ad8p-33", "%-22.6a", 0.000000000123);
  // TR_FORMAT_ASSERT("0x000000000001.0e7ad8p-33", "%022.6a", 0.000000000123);

  // 0x1.8p-1
  // 0x1.90000p-1
  // 0x0p+0
  // 0x1p-33

  // printf("%a\n", 0.75);
  // printf("%0.5a\n", 0.78125);
  // printf("%a\n", 0.0);
  // printf("%.0a\n", 0.000000000123);
}
