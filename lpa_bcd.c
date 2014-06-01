/*

Binary coded decimal large precision arithmetic functions

*/

#include <string.h>
#include <stdio.h>

#include "lpa_bcd.h"

#define LPA_BCD_DIGIT_MASK (0xF)

#define LPA_BCD_DEBUG (0)

#if LPA_BCD_DEBUG
#define LPA_DEBUG printf
#else
#define LPA_DEBUG if(0) printf
#endif
/*

Private functions

*/

static void LPA_BCDextendNumber(LPA_BCDnumber* const pNumber, const LPA_BCD_size sizeIncrement)
{
	if (pNumber == NULL)
	{
		LPA_ERROR("LPA_BCDextendNumber::pNumber is NULL");
		return;
	}
	else
	{
		const LPA_BCD_digit* const pOldDigits = pNumber->pDigits;
		const LPA_BCD_size oldMemorySize = pNumber->memorySize;
		const LPA_BCD_size newMemorySize = pNumber->memorySize+sizeIncrement;

		pNumber->pDigits = LPA_allocMem(newMemorySize);
		pNumber->memorySize = newMemorySize;
		memcpy(pNumber->pDigits, pOldDigits, oldMemorySize);
		LPA_freeMem((void*)pOldDigits);
	}
}

static void LPA_BCDinitNumber(LPA_BCDnumber* const pNumber)
{
	pNumber->pDigits = NULL;
	pNumber->memorySize = 0;
}

static void LPA_BCDallocNumber(LPA_BCDnumber* const pNumber, const LPA_BCD_size newMemorySize)
{
	if (pNumber == NULL)
	{
		LPA_ERROR("LPA_BCDallocNumber::pNumber is NULL");
		return;
	}
	if (newMemorySize > pNumber->memorySize)
	{
		LPA_freeMem(pNumber->pDigits);
		pNumber->pDigits = LPA_allocMem(newMemorySize);
		pNumber->memorySize = newMemorySize;
	}
}

/*

Public functions

*/

void LPA_BCDsprintf(const LPA_BCDnumber* const pNumber, char* const pBuffer, const size_t maxNumChars)
{
	LPA_BCD_size i;
	size_t outIndex = 0;
	const LPA_BCD_size maxLoop = pNumber->memorySize;
	LPA_BCD_digitIntermediate digit = 0;

	for (i = 0; i < maxLoop; ++i)
	{
		unsigned int j = 0;
		digit = pNumber->pDigits[i];

		for (j = 0; j < 2; ++j)
		{
			LPA_BCD_digitIntermediate shift = (j << 2);
			const LPA_BCD_digitIntermediate value = (digit >> shift) & LPA_BCD_DIGIT_MASK;
			LPA_DEBUG("digit:0x%X shift:%d value:%u\n", digit, shift, value);
			/* ignore 0 if it is the final character */
			if ((i == maxLoop-1) && (j == 2))
			{
				if (value == 0)
				{
					break;
				}
			}
			pBuffer[outIndex] = (char)('0' + value);
			++outIndex;
			if (outIndex == maxNumChars)
			{
				break;
			}
		}
	}
	pBuffer[outIndex] = '\0';

	/* reverse the string to make it correct for printing */
	/* 		could do this in the above loop but would need to compute number of chars in the output which is a more complex loop */
	for (i = 0; i < outIndex/2; ++i)
	{
		char c = pBuffer[i];
		pBuffer[i] = pBuffer[outIndex-1-i];
		pBuffer[outIndex-1-i] = c;
	}
}

void LPA_BCDcreateNumber(LPA_BCDnumber* const pNumber, LPA_uint32 value)
{
	LPA_uint32 workingValue = value;
	LPA_BCD_size index = 0;
	LPA_BCD_size nibbleIndex = 0;

	LPA_BCDinitNumber(pNumber);

	while (workingValue != 0)
	{
		LPA_BCD_digitIntermediate digit = workingValue % 10;
		LPA_DEBUG("workingValue:%u index:%d nibbleIndex:%d digit:%u\n", workingValue, index, nibbleIndex, digit);
		if (nibbleIndex == 0)
		{
			LPA_BCDextendNumber(pNumber, 1);
			pNumber->pDigits[index] = 0;
		}
		digit = digit << (nibbleIndex * 4);
		pNumber->pDigits[index] |= (LPA_BCD_digit)digit;
		nibbleIndex ^= 1;
		if (nibbleIndex == 0)
		{
			++index;
		}

		workingValue /= 10;
	}
}

void LPA_BCDadd(const LPA_BCDnumber* const pA, const LPA_BCDnumber* const pB, LPA_BCDnumber* const pSum)
{
	LPA_BCD_size i = 0;
	const LPA_BCD_size aMemSize = pA->memorySize;
	const LPA_BCD_size bMemSize = pB->memorySize;
	/* sum length : extend if needed for the carry */
	const LPA_BCD_size sumSize = ((aMemSize > bMemSize) ? aMemSize : bMemSize);
	LPA_BCD_digit carry = 0;

	LPA_BCDallocNumber(pSum, sumSize);

	for (i = 0; i < sumSize; ++i)
	{
		unsigned int j = 0;
		
		LPA_BCD_digitIntermediate aDigit = 0;
		LPA_BCD_digitIntermediate bDigit = 0;
		LPA_BCD_digitIntermediate sumDigit = 0;

		if (i < aMemSize)
		{
			aDigit = pA->pDigits[i];
		}
		if (i < bMemSize)
		{
			bDigit = pB->pDigits[i];
		}
		for (j = 0; j < 2; ++j)
		{
			LPA_BCD_digitIntermediate shift = (j << 2);
			const LPA_BCD_digitIntermediate aValue = (aDigit >> shift) % 10;
			const LPA_BCD_digitIntermediate bValue = (bDigit >> shift) % 10;
			LPA_BCD_digitIntermediate sumValue = aValue + bValue;
			sumValue += carry;

			if (sumValue > 9)
			{
				sumValue -= 10;
				carry = 1;
			}
			else
			{
				carry = 0;
			}
			sumDigit |= (sumValue << shift);
		}
	}
	/* Extend sum if is set */
	if (carry == 1)
	{
		LPA_BCDextendNumber(pSum, 1);
		pSum->pDigits[sumSize] = 0x1;
	}
}

/*
Functions that will be needed
*/

