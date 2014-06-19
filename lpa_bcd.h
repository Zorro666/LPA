#ifndef LPA_BCD_H
#define LPA_BCD_H

#include "lpa_core.h"

typedef LPA_uint32 LPA_BCD_size;
typedef LPA_uint8 LPA_BCD_digit;

/* 

LPA_BCD_number : each byte is 2 digits (4-bits per digit)
Could use a length member or could have a sentinel e.g. 0xFF to mark end of digits
Using a sentinel would mean pDigits has to be allocated at all times
Using a length member means we have to have some kind a create/init function
Using a length member limits the size of the number to (2^32-1)*2 digits or (2^64-1)*2 if we use a 64-bit int for length and so on

Negative numbers handled as a sign flag and storing the absolute value

*/

typedef struct LPA_BCD_number
{
	LPA_BCD_digit* pDigits; /* each LPA_BCD_digit is 2 decimal numbers */
	LPA_BCD_size numDigits; /* could store the number of decimal digits instead/as well */
	int negative;
} LPA_BCD_number;

void LPA_BCD_initNumber(LPA_BCD_number* const pNumber);
void LPA_BCD_freeNumber(LPA_BCD_number* const pNumber);

/* Do we need FromUint32 & FromUint64 etc., could we just have FromUint */
void LPA_BCD_fromInt32(LPA_BCD_number* const pNumber, LPA_int32 value);
void LPA_BCD_fromInt64(LPA_BCD_number* const pNumber, LPA_int64 value);
void LPA_BCD_fromUint32(LPA_BCD_number* const pNumber, LPA_uint32 value);
void LPA_BCD_fromUint64(LPA_BCD_number* const pNumber, LPA_uint64 value);

/* Decimal ASCII only (signed or unsigned) */
void LPA_BCD_fromDecimalASCII(LPA_BCD_number* const pNumber, const char* const value);

void LPA_BCD_toDecimalASCII(char* const pBuffer, const size_t maxNumChars, const LPA_BCD_number* const pNumber);
void LPA_BCD_toHexadecimalASCII(char* const pBuffer, const size_t maxNumChars, const LPA_BCD_number* const pNumber);

void LPA_BCD_add(LPA_BCD_number* const pResult, const LPA_BCD_number* const pA, const LPA_BCD_number* const pB);
void LPA_BCD_subtract(LPA_BCD_number* const pResult, const LPA_BCD_number* const pA, const LPA_BCD_number* const pB);
void LPA_BCD_multiply(LPA_BCD_number* const pResult, const LPA_BCD_number* const pA, const LPA_BCD_number* const pB);
void LPA_BCD_divide(LPA_BCD_number* const pQuotient, LPA_BCD_number* const pRemainder, const LPA_BCD_number* const pA, const LPA_BCD_number* const pB);

#endif
