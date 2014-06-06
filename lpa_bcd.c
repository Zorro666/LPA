/*

Binary coded decimal large precision arithmetic functions

*/

#include <string.h>
#include <stdio.h>

#include "lpa_bcd.h"

#define LPA_BCD_DIGIT_MASK (0xF)

#define LPA_BCD_DEBUG (1)

#if LPA_BCD_DEBUG
#define LPA_BCD_LOG LPA_LOG
#else
#define LPA_BCD_LOG if (0) LPA_LOG
#endif 

/*

Private functions

*/

static void LPA_BCDinvert(LPA_BCDnumber* const pNumber)
{
	/* loop over the digits: (9-digit) for all digits except units digit which is (10-digit) */
	LPA_BCD_size i;
	const LPA_BCD_size maxLoop = pNumber->memorySize;

	for (i = 0; i < maxLoop; i++)
	{
		const LPA_BCD_digitIntermediate digit = pNumber->pDigits[i];
		LPA_BCD_digit outDigit = 0;
		unsigned int j = 0;
		for (j = 0; j < 2; ++j)
		{
			const LPA_BCD_digitIntermediate shift = (j << 2);
			const LPA_BCD_digitIntermediate value = (digit >> shift) & LPA_BCD_DIGIT_MASK;
			LPA_BCD_digitIntermediate outValue = 9 - value;
			if ((i == 0) && (j == 0))
			{
				outValue++;
			}
			LPA_BCD_LOG("i:%u j:%u value:%d outValue:%d\n", i, j, value, outValue);
			outDigit |= (LPA_BCD_digit)(outValue << shift);
		}
		pNumber->pDigits[i] = outDigit;
	}
	/* Invert the -ve flag */
	pNumber->negative ^= 1;
}

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
		const LPA_BCD_size newMemorySize = pNumber->memorySize+(LPA_BCD_size)(sizeof(LPA_BCD_digit)*sizeIncrement);

		pNumber->pDigits = LPA_allocMem(newMemorySize);
		pNumber->memorySize = newMemorySize;
		memcpy(pNumber->pDigits, pOldDigits, oldMemorySize);
		LPA_freeMem((void*)pOldDigits);
	}
}

static void LPA_BCDallocNumber(LPA_BCDnumber* const pNumber, const LPA_BCD_size newMemorySize)
{
	if (pNumber == NULL)
	{
		LPA_ERROR("LPA_BCDallocNumber::pNumber is NULL");
		return;
	}
	if (newMemorySize != pNumber->memorySize)
	{
		LPA_freeMem(pNumber->pDigits);
		pNumber->pDigits = LPA_allocMem(newMemorySize);
		pNumber->memorySize = newMemorySize;
		memset(pNumber->pDigits, 0, newMemorySize);
	}
}

static void LPA_BCDaddInternal(const LPA_BCDnumber* const pA, const LPA_BCDnumber* const pB, LPA_BCDnumber* const pResult)
{
	LPA_BCD_size i = 0;
	const LPA_BCD_size aMemSize = pA->memorySize;
	const LPA_BCD_size bMemSize = pB->memorySize;
	/* sum length : extend if needed for the carry */
	const LPA_BCD_size sumSize = ((aMemSize > bMemSize) ? aMemSize : bMemSize);
	LPA_BCD_digit carry = 0;

	LPA_BCD_LOG("sumSize:%d\n", sumSize);
	LPA_BCDallocNumber(pResult, sumSize);

	for (i = 0; i < sumSize; ++i)
	{
		unsigned int j = 0;
		
		LPA_BCD_digitIntermediate aDigit = 0;
		LPA_BCD_digitIntermediate bDigit = 0;
		LPA_BCD_digit sumDigit = 0;
		LPA_BCD_LOG("i:%d\n", i);

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
			const LPA_BCD_digitIntermediate aValue = (aDigit >> shift) & LPA_BCD_DIGIT_MASK;
			const LPA_BCD_digitIntermediate bValue = (bDigit >> shift) & LPA_BCD_DIGIT_MASK;
			LPA_BCD_digitIntermediate sumValue = aValue + bValue;
			sumValue += carry;
			LPA_BCD_LOG("carry:%u sumValue:%u aValue:%u bValue:%u\n", carry, sumValue, aValue, bValue);

			if (sumValue > 9)
			{
				sumValue -= 10;
				carry = 1;
			}
			else
			{
				carry = 0;
			}
			sumDigit |= (LPA_BCD_digit)(sumValue << shift);
		}
		pResult->pDigits[i] = sumDigit;
	}
	/* Extend sum if carry is set */
	if (carry == 1)
	{
		LPA_BCDextendNumber(pResult, 1);
		pResult->pDigits[sumSize] = 0x1;
	}
}

static void LPA_BCDsubtractInternal(const LPA_BCDnumber* const pA, const LPA_BCDnumber* const pB, LPA_BCDnumber* const pResult)
{
	LPA_BCD_size i = 0;
	const LPA_BCD_size aMemSize = pA->memorySize;
	const LPA_BCD_size bMemSize = pB->memorySize;
	/* sum length : extend if needed for the carry */
	const LPA_BCD_size sumSize = ((aMemSize > bMemSize) ? aMemSize : bMemSize);
	LPA_BCD_digit carry = 0;

	LPA_BCD_LOG("sumSize:%d\n", sumSize);
	LPA_BCDallocNumber(pResult, sumSize);

	for (i = 0; i < sumSize; ++i)
	{
		unsigned int j = 0;
		
		LPA_BCD_digitIntermediate aDigit = 0;
		LPA_BCD_digitIntermediate bDigit = 0;
		LPA_BCD_digit sumDigit = 0;
		LPA_BCD_LOG("i:%d\n", i);

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
			const LPA_BCD_digitIntermediate aValue = (aDigit >> shift) & LPA_BCD_DIGIT_MASK;
			const LPA_BCD_digitIntermediate bValue = (bDigit >> shift) & LPA_BCD_DIGIT_MASK;
			LPA_BCD_digitIntermediate sumValue = aValue - bValue;
			sumValue -= carry;
			LPA_BCD_LOG("j:%d carry:%u sumValue:%u aValue:%u bValue:%u\n", j, carry, sumValue, aValue, bValue);

			if (sumValue > 9)
			{
				sumValue += 10;
				carry = 1;
				LPA_BCD_LOG("j:%d 2 carry:%u sumValue:%u aValue:%u bValue:%u\n", j, carry, sumValue, aValue, bValue);
			}
			else
			{
				carry = 0;
			}
			sumDigit |= (LPA_BCD_digit)(sumValue << shift);
		}
		pResult->pDigits[i] = sumDigit;
	}
	pResult->negative = 0;
	/* if carry is set then this is a -ve number */
	if (carry == 1)
	{
		LPA_BCDinvert(pResult);
	}
}

/*

Public functions

*/

void LPA_BCDinitNumber(LPA_BCDnumber* const pNumber)
{
	pNumber->pDigits = NULL;
	pNumber->memorySize = 0;
	pNumber->negative = 0;
}

void LPA_BCDsprintf(const LPA_BCDnumber* const pNumber, char* const pBuffer, const size_t maxNumChars)
{
	LPA_BCD_size i;
	size_t outIndex = 0;
	const LPA_BCD_size maxLoop = pNumber->memorySize;
	LPA_BCD_digitIntermediate digit = 0;
	size_t maxValidChar = 0;

	if (maxLoop == 0)
	{
		pBuffer[0] = '\0';
		return;
	}
	LPA_BCD_LOG("maxLoop:%u\n", maxLoop);
	for (i = 0; i < maxLoop; ++i)
	{
		unsigned int j = 0;
		digit = pNumber->pDigits[i];

		for (j = 0; j < 2; ++j)
		{
			LPA_BCD_digitIntermediate shift = (j << 2);
			const LPA_BCD_digitIntermediate value = (digit >> shift) & LPA_BCD_DIGIT_MASK;
			LPA_BCD_LOG("digit:0x%X shift:%d value:%u\n", digit, shift, value);
			pBuffer[outIndex] = (char)('0' + value);
			if (value > 0)
			{
				maxValidChar = outIndex;
			}
			++outIndex;
			if (outIndex == maxNumChars)
			{
				--outIndex;
				break;
			}
		}
	}
	outIndex = maxValidChar+1;
	if (pNumber->negative)
	{
		if (outIndex == maxNumChars-1)
		{
			outIndex--;
		}
		pBuffer[outIndex] = '-';
		outIndex++;
	}
	pBuffer[outIndex] = '\0';

	/* reverse the string to make it correct for printing could do this in the above loop, */
 	/* but then would need to compute the number of characters in the output which is a more complex loop than a simple reverse */
	for (i = 0; i < outIndex/2; ++i)
	{
		char c = pBuffer[outIndex-1-i];
		pBuffer[outIndex-1-i] = pBuffer[i];
		pBuffer[i] = c;
	}
}

/* Decimal ASCII only */
void LPA_BCDcreateNumberFromASCII(LPA_BCDnumber* const pNumber, const char* const value)
{
	const char* pEnd;
	const char* pStr = value;
	LPA_BCD_size index = 0;
	LPA_BCD_size nibbleIndex = 0;
	int negative = 0;

	LPA_BCDinitNumber(pNumber);
	if (pStr == NULL)
	{
		return;
	}
	if (*pStr == '\0')
	{
		return;
	}

	while (*pStr != '\0')
	{
		const char c = *pStr;
		if ((c == '-') || (c == '+') || ((c >= '0') && (c <= '9')))
		{
			break;
		}
		++pStr;
	}

	if (*pStr == '-')
	{
		negative = 1;
		++pStr;
	}
	else if (*pStr == '+')
	{
		negative = 0;
		++pStr;
	}
	pEnd = pStr;

	while (*pStr != '\0')
	{
		++pStr;
	}

	do
	{
		const char c = *--pStr;
		LPA_BCD_digitIntermediate digit = (LPA_BCD_digitIntermediate)(c - '0');
		if (digit > 9)
		{
			LPA_freeMem(pNumber->pDigits);
			LPA_BCDinitNumber(pNumber);
			return;
		}
		LPA_BCD_LOG("c:%c index:%d nibbleIndex:%d digit:%u\n", c, index, nibbleIndex, digit);
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
	}
	while (pStr != pEnd);
	pNumber->negative = negative;
}

void LPA_BCDcreateNumberFromInt32(LPA_BCDnumber* const pNumber, LPA_int32 value)
{
	LPA_BCDcreateNumberFromInt64(pNumber, value);
}

void LPA_BCDcreateNumberFromInt64(LPA_BCDnumber* const pNumber, LPA_int64 value)
{
	if (value < 0)
	{
		LPA_int64 absValue = -value;
		LPA_BCDcreateNumberFromUint64(pNumber, (LPA_uint64)absValue);
		pNumber->negative = 1;
	}
	else
	{
		LPA_BCDcreateNumberFromUint64(pNumber, (LPA_uint64)value);
	}
}

void LPA_BCDcreateNumberFromUint32(LPA_BCDnumber* const pNumber, LPA_uint32 value)
{
	LPA_BCDcreateNumberFromUint64(pNumber, value);
}

void LPA_BCDcreateNumberFromUint64(LPA_BCDnumber* const pNumber, LPA_uint64 value)
{
	LPA_uint64 workingValue = value;
	LPA_BCD_size index = 0;
	LPA_BCD_size nibbleIndex = 0;

	LPA_BCDinitNumber(pNumber);

	while (workingValue != 0)
	{
		LPA_BCD_digitIntermediate digit = (LPA_BCD_digitIntermediate)(workingValue % 10);
		LPA_BCD_LOG("workingValue:%lu index:%d nibbleIndex:%d digit:%u\n", workingValue, index, nibbleIndex, digit);
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

void LPA_BCDadd(const LPA_BCDnumber* const pA, const LPA_BCDnumber* const pB, LPA_BCDnumber* const pResult)
{
	if (pA->negative)
	{
		if (pB->negative)
		{
			LPA_BCDaddInternal(pA, pB, pResult);
			pResult->negative = 1;
			return;
		}
		else
		{
			LPA_BCDsubtractInternal(pB, pA, pResult);
			return;
		}
	}
	else if (pB->negative)
	{
		LPA_BCDsubtractInternal(pA, pB, pResult);
		return;
	}
	LPA_BCDaddInternal(pA, pB, pResult);
}

void LPA_BCDsubtract(const LPA_BCDnumber* const pA, const LPA_BCDnumber* const pB, LPA_BCDnumber* const pResult)
{
	if (pA->negative)
	{
		if (pB->negative)
		{
			LPA_BCDsubtractInternal(pB, pA, pResult);
			return;
		}
		else
		{
			LPA_BCDaddInternal(pA, pB, pResult);
			pResult->negative = 1;
			return;
		}
	}
	else if (pB->negative)
	{
		LPA_BCDaddInternal(pA, pB, pResult);
		pResult->negative = 0;
		return;
	}
	LPA_BCDsubtractInternal(pA, pB, pResult);
}

