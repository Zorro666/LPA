#ifndef LPA_BCD_H
#define LPA_BCD_H

#include "lpa_core.h"

typedef LPA_uint32 LPA_BCD_size;
typedef LPA_uint8 LPA_BCD_digit;

/* 

LPA_BCDnumber : each byte is 2 digits (4-bits per digit)
Could use a length member or could have a sentinel e.g. 0xFF to mark end of digits
Using a sentinel would mean pDigits has to be allocated at all times
Using a length member means we have to have some kind a create/init function
Using a length member limits the size of the number to (2^32-1)*2 digits or (2^64-1)*2 if we use a 64-bit int for length and so on

Negative numbers handled as a sign flag and storing the absolute value

*/

typedef struct LPA_BCDnumber
{
	LPA_BCD_digit* pDigits; /* each LPA_BCD_digit is 2 decimal numbers */
	LPA_BCD_size numDigits;
	int negative;
} LPA_BCDnumber;

void LPA_BCDinitNumber(LPA_BCDnumber* const pNumber);

/* Do we need FromUint32 & FromUint64 etc., could we just have FromUint */
void LPA_BCDcreateNumberFromInt32(LPA_BCDnumber* const pNumber, LPA_int32 value);
void LPA_BCDcreateNumberFromInt64(LPA_BCDnumber* const pNumber, LPA_int64 value);
void LPA_BCDcreateNumberFromUint32(LPA_BCDnumber* const pNumber, LPA_uint32 value);
void LPA_BCDcreateNumberFromUint64(LPA_BCDnumber* const pNumber, LPA_uint64 value);

/* Decimal ASCII only (signed or unsigned) */
void LPA_BCDcreateNumberFromASCII(LPA_BCDnumber* const pNumber, const char* const value);

void LPA_BCDsprintf(const LPA_BCDnumber* const pNumber, char* const pBuffer, const size_t maxNumChars);

void LPA_BCDadd(const LPA_BCDnumber* const pA, const LPA_BCDnumber* const pB, LPA_BCDnumber* const pResult);
void LPA_BCDsubtract(const LPA_BCDnumber* const pA, const LPA_BCDnumber* const pB, LPA_BCDnumber* const pResult);

#endif
