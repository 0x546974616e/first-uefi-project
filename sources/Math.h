#ifndef TR_MATH_H
#define TR_MATH_H

#include "Helper.h"

// IEEE 754, ISO/IEC 60559:
//    63   62        52 51       0
// ├──────┼────────────┼──────────┤
// │ Sign │ Expononent │ Mantissa │
// └──────┴────────────┴──────────┘

#define TR_DOUBLE_MANTISSA_MASK ((UINT64) 0x000FFFFFFFFFFFFFU)
#define TR_DOUBLE_EXPONENT_MASK ((UINT64) 0x7FF0000000000000U)

DOUBLE TrLog10Double(DOUBLE Value);
DOUBLE TrPow10Double(DOUBLE Value);

#endif // TR_MATH_H
