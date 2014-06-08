#ifndef LPA_INT_H
#define LPA_INT_H

#include "lpa_core.h"

typedef LPA_uint32 LPA_INT_size;
typedef LPA_uint32 LPA_INT_digit;

/* 

LPA_INT_number : each 32-bits is 1 digit

*/

typedef struct LPA_INT_number
{
	LPA_INT_digit* pDigits;
	LPA_INT_size numDigits;
} LPA_INT_number;

void LPA_INT_initNumber(LPA_INT_number* const pNumber);

/* Do we need FromUint32 & FromUint64 etc., could we just have FromUint */
void LPA_INT_fromInt32(LPA_INT_number* const pNumber, LPA_int32 value);
void LPA_INT_fromInt64(LPA_INT_number* const pNumber, LPA_int64 value);
void LPA_INT_fromUint32(LPA_INT_number* const pNumber, LPA_uint32 value);
void LPA_INT_fromUint64(LPA_INT_number* const pNumber, LPA_uint64 value);

/* Hexadecimal ASCII only (signed) */
void LPA_INT_fromHexadecimalASCII(LPA_INT_number* const pNumber, const char* const value);

void LPA_INT_toHexadecimalASCII(const LPA_INT_number* const pNumber, char* const pBuffer, const size_t maxNumChars);

void LPA_INT_add(const LPA_INT_number* const pA, const LPA_INT_number* const pB, LPA_INT_number* const pResult);
void LPA_INT_subtract(const LPA_INT_number* const pA, const LPA_INT_number* const pB, LPA_INT_number* const pResult);

#endif
