#include "lpa_core.h"

typedef LPA_uint32 LPA_BCD_size;
typedef LPA_uint32 LPA_BCD_digitIntermediate;
typedef LPA_uint8 LPA_BCD_digit;

/* 
LPA_BCDnumber : each byte is 2 digits (4-bits per digit)
Could use a length member or could have a sentinel e.g. 0xFF to mark end of digits
Using a sentinel would mean pDigits has to be allocated at all times
Using a length member means we have to have some kind a create/init function
Using a length member limits the size of the number to (2^32-1)*2 digits or (2^64-1)*2 if we use a 64-bit int for length and so on

Need to decide about how want to handle -ve numbers
*/

typedef struct LPA_BCDnumber
{
	LPA_BCD_digit* pDigits;
	LPA_BCD_size memorySize; /* this is the memory size = 2 * the number of digits */
} LPA_BCDnumber;

void LPA_BCDcreateNumber(LPA_BCDnumber* const pNumber, LPA_uint32 value);
void LPA_BCDsprintf(const LPA_BCDnumber* const pNumber, char* const pBuffer, const size_t maxNumChars);

void LPA_BCDadd(const LPA_BCDnumber* const pA, const LPA_BCDnumber* const pB, LPA_BCDnumber* const pSum);

/*
Functions that will be needed
Convert BCD to char* for printing
*/
