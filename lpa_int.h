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

void LPA_INTinitNumber(LPA_INTnumber* const pNumber);

/* Do we need FromUint32 & FromUint64 etc., could we just have FromUint */
void LPA_INTcreateNumberFromInt32(LPA_INTnumber* const pNumber, LPA_int32 value);
void LPA_INTcreateNumberFromInt64(LPA_INTnumber* const pNumber, LPA_int64 value);
void LPA_INTcreateNumberFromUint32(LPA_INTnumber* const pNumber, LPA_uint32 value);
void LPA_INTcreateNumberFromUint64(LPA_INTnumber* const pNumber, LPA_uint64 value);

/* Decimal ASCII only (signed or unsigned) */
void LPA_INTcreateNumberFromASCII(LPA_INTnumber* const pNumber, const char* const value);

void LPA_INTsprintf(const LPA_INTnumber* const pNumber, char* const pBuffer, const size_t maxNumChars);

void LPA_INTadd(const LPA_INTnumber* const pA, const LPA_INTnumber* const pB, LPA_INTnumber* const pResult);
void LPA_INTsubtract(const LPA_INTnumber* const pA, const LPA_INTnumber* const pB, LPA_INTnumber* const pResult);

#endif
