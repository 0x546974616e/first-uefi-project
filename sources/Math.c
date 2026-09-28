#include "Math.h"
#include "EfiBase.h"

STATIC CONST DOUBLE
  LOG10_20 = 0.30102999566398119521373,
  LOG10_15 = 0.17609125905568124208128,
  LDX10_15 = 0.28952965460216788510075;

// Inspired by <https://www.ampl.com/netlib/fp/dtoa.c>.
DOUBLE TrLog10Double(DOUBLE Value) {
  // Let 𝑚 = 1 + Mantissa, 𝑒 = Exponent - 1023 and 𝑥 = 𝑚 × 2^𝑒
  //   where 𝑚 ∈ [1, 2) and 𝑒 ∈ [−1022, +1023].
  // It follows that log10(𝑥) = log10(𝑚) + 𝑒 × log10(2).
  // We can approximate log10(𝑚) by linearization around 1.5 with
  //   𝑎 × (𝑚 − 1.5) + log10(1.5) where 𝑎 = 1 ÷ (1.5 × ln(10)).

  union { DOUBLE D; UINT64 U; } Ieee754 = { Value };
  INT64 Exponent = (INT64)((Ieee754.U & TR_DOUBLE_EXPONENT_MASK) >> 52U) - 1023LL;
  Ieee754.U = (Ieee754.U & TR_DOUBLE_MANTISSA_MASK) | (1023ULL << 52U); // [1, 2)
  return LDX10_15 * (Ieee754.D - 1.5) + LOG10_15 + ((DOUBLE) Exponent * LOG10_20);
}

DOUBLE TrPow10Double(DOUBLE Value) {
  // TODO TMP WIP
  return Value;
}
