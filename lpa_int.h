#ifndef LPA_INT_H
#define LPA_INT_H

#include "lpa_core.h"

typedef LPA_uint32 LPA_INT_size;
typedef LPA_uint32 LPA_INT_digit;

/* 

LPA_INTnumber : each 32-bits is 1 digit

*/

typedef struct LPA_INTnumber
{
	LPA_INT_digit* pDigits;
	LPA_INT_size numDigits;
} LPA_INTnumber;

void LPA_INT_initNumber(LPA_INTnumber* const pNumber);

/* Do we need FromUint32 & FromUint64 etc., could we just have FromUint */
void LPA_INT_fromInt32(LPA_INTnumber* const pNumber, LPA_int32 value);
void LPA_INT_fromInt64(LPA_INTnumber* const pNumber, LPA_int64 value);
void LPA_INT_fromUint32(LPA_INTnumber* const pNumber, LPA_uint32 value);
void LPA_INT_fromUint64(LPA_INTnumber* const pNumber, LPA_uint64 value);

/* Hexadecimal ASCII only (signed) */
void LPA_INT_fromHexadecimalASCII(LPA_INTnumber* const pNumber, const char* const value);

void LPA_INT_toHexadecimalASCII(const LPA_INTnumber* const pNumber, char* const pBuffer, const size_t maxNumChars);

void LPA_INT_add(const LPA_INTnumber* const pA, const LPA_INTnumber* const pB, LPA_INTnumber* const pResult);
void LPA_INT_subtract(const LPA_INTnumber* const pA, const LPA_INTnumber* const pB, LPA_INTnumber* const pResult);

#endif
