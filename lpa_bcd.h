#include "lpa_core.h"

typedef LPA_uint32 LPA_BCD_size;
/* 
 * LPA_BCDnumber : each byte is 2 digits (4-bits per digit)
 * Could use a length member or could have a sentinel e.g. 0xFF to mark end of digits
 * Using a sentinel would mean digits has to be allocated at all times
 * Using a length member means have to some a create/init function
 * Using a length member limits the size of the number to (2^32-1)*2 digits or (2^64-1)*2 if use a 64-bit int for length and so on
 */

typedef struct LPA_BCDnumber
{
	LPA_uint8* pDigits;
	LPA_BCD_size memorySize; /* this is the memory size = 2 * the number of digits */
} LPA_BCDnumber;

void LPA_BCDinitNumber(LPA_BCDnumber* const pNumber);

void LPA_BCDallocNumber(LPA_BCDnumber* const pNumber, const LPA_BCD_size newMemorySize);

void LPA_BCDadd(const LPA_BCDnumber* const pA, const LPA_BCDnumber* pB, LPA_BCDnumber* const pSum);

/*
 * Functions that will be needed
 * Convert from int/long -> BCD number
 * Convert BCD to char* for printing
 */
