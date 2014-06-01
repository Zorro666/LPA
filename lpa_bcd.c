/*
Binary coded decimal large precision arithmetic functions
*/

#include "lpa_bcd.h"

void LPA_BCDinitNumber(LPA_BCDnumber* const pNumber)
{
	pNumber->pDigits = NULL;
	pNumber->memorySize = 0;
}

void LPA_BCDallocNumber(LPA_BCDnumber* const pNumber, const LPA_BCD_size newMemorySize)
{
	if (pNumber == NULL)
	{
		LPA_ERROR("pNumber is NULL");
		return;
	}
	if (newMemorySize > pNumber->memorySize)
	{
		LPA_freeMem(pNumber->pDigits);
		pNumber->pDigits = LPA_allocMem(newMemorySize);
		pNumber->memorySize = newMemorySize;
	}
}

void LPA_BCDadd(const LPA_BCDnumber* const pA, const LPA_BCDnumber* pB, LPA_BCDnumber* const pSum)
{
	const LPA_BCD_size aMemSize = pA->memorySize;
	const LPA_BCD_size bMemSize = pB->memorySize;
	/* Overestimate for sum length */
	const LPA_BCD_size sumSize = ((aMemSize > bMemSize) ? aMemSize : bMemSize) + 1;

	LPA_BCDallocNumber(pSum, sumSize);
}

/*
 * Functions that will be needed
 * Convert from int/long -> BCD number
 * Convert BCD to char* for printing
 */

