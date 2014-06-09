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

typedef LPA_int32 LPA_BCD_digitIntermediate;

/*

Private functions

*/

static void LPA_BCD_invert(LPA_BCD_number* const pNumber)
{
	/* loop over the digits: 10^(n+1) - number */
	LPA_BCD_size i;
	const LPA_BCD_size maxLoop = pNumber->numDigits;
	LPA_BCD_digitIntermediate carry = 1;

	for (i = 0; i < maxLoop; i++)
	{
		const LPA_BCD_digitIntermediate digit = pNumber->pDigits[i];
		LPA_BCD_digit outDigit = 0;
		unsigned int j = 0;
		for (j = 0; j < 2; ++j)
		{
			const LPA_BCD_digitIntermediate shift = (LPA_BCD_digitIntermediate)(j << 2);
			const LPA_BCD_digitIntermediate value = (LPA_BCD_digitIntermediate)(digit >> shift) & LPA_BCD_DIGIT_MASK;
			LPA_BCD_digitIntermediate outValue = 9 - value;

			outValue += carry;
			if (outValue > 9)
			{
				carry = 1;
				outValue -= 10;
			}
			else
			{
				carry = 0;
			}
			LPA_BCD_LOG("i:%u j:%u value:%d outValue:%d\n", i, j, value, outValue);
			outDigit |= (LPA_BCD_digit)(outValue << shift);
		}
		pNumber->pDigits[i] = outDigit;
	}
	/* Invert the -ve flag */
	pNumber->negative ^= 1;
}

static void LPA_BCD_extendNumber(LPA_BCD_number* const pNumber, const LPA_BCD_size sizeIncrement)
{
	if (pNumber == NULL)
	{
		LPA_ERROR("LPA_BCD_extendNumber::pNumber is NULL");
		return;
	}
	else
	{
		const LPA_BCD_digit* const pOldDigits = pNumber->pDigits;
		const LPA_BCD_size oldNumDigits = pNumber->numDigits;
		const LPA_BCD_size newNumDigits = oldNumDigits + sizeIncrement;
		const size_t oldMemorySize = oldNumDigits * sizeof(LPA_BCD_digit);
		const size_t newMemorySize = newNumDigits * sizeof(LPA_BCD_digit);

		pNumber->pDigits = LPA_allocMem(newMemorySize);
		pNumber->numDigits = newNumDigits;
		memcpy(pNumber->pDigits, pOldDigits, oldMemorySize);
		LPA_freeMem((void*)pOldDigits);
	}
}

static void LPA_BCD_allocNumber(LPA_BCD_number* const pNumber, const LPA_BCD_size newNumDigits)
{
	if (pNumber == NULL)
	{
		LPA_ERROR("LPA_BCD_allocNumber::pNumber is NULL");
		return;
	}
	if (newNumDigits != pNumber->numDigits)
	{
		const size_t newMemorySize = newNumDigits * sizeof(LPA_BCD_digit);
		LPA_freeMem(pNumber->pDigits);
		pNumber->pDigits = LPA_allocMem(newMemorySize);
		pNumber->numDigits = newNumDigits;
		memset(pNumber->pDigits, 0, newMemorySize);
	}
	pNumber->negative = 0;
}

static void LPA_BCD_addInternal(const LPA_BCD_number* const pA, const LPA_BCD_number* const pB, LPA_BCD_number* const pResult)
{
	LPA_BCD_size i = 0;
	const LPA_BCD_size aNumDigits = pA->numDigits;
	const LPA_BCD_size bNumDigits = pB->numDigits;
	/* sum length : extend if needed for the carry */
	const LPA_BCD_size sumSize = ((aNumDigits > bNumDigits) ? aNumDigits : bNumDigits);
	LPA_BCD_digit carry = 0;

	LPA_BCD_LOG("sumSize:%d\n", sumSize);
	LPA_BCD_allocNumber(pResult, sumSize);

	for (i = 0; i < sumSize; ++i)
	{
		unsigned int j = 0;
		
		LPA_BCD_digitIntermediate aDigit = 0;
		LPA_BCD_digitIntermediate bDigit = 0;
		LPA_BCD_digit sumDigit = 0;
		LPA_BCD_LOG("i:%d\n", i);

		if (i < aNumDigits)
		{
			aDigit = pA->pDigits[i];
		}
		if (i < bNumDigits)
		{
			bDigit = pB->pDigits[i];
		}
		for (j = 0; j < 2; ++j)
		{
			const LPA_BCD_digitIntermediate shift = (LPA_BCD_digitIntermediate)(j << 2);
			const LPA_BCD_digitIntermediate aValue = (aDigit >> shift) & LPA_BCD_DIGIT_MASK;
			const LPA_BCD_digitIntermediate bValue = (bDigit >> shift) & LPA_BCD_DIGIT_MASK;
			LPA_BCD_digitIntermediate sumValue = aValue + bValue + carry;
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
		LPA_BCD_extendNumber(pResult, 1);
		pResult->pDigits[sumSize] = 0x1;
	}
}

static void LPA_BCD_subtractInternal(const LPA_BCD_number* const pA, const LPA_BCD_number* const pB, LPA_BCD_number* const pResult)
{
	LPA_BCD_size i = 0;
	const LPA_BCD_size aNumDigits = pA->numDigits;
	const LPA_BCD_size bNumDigits = pB->numDigits;
	/* sum length : extend if needed for the borrow */
	const LPA_BCD_size sumSize = ((aNumDigits > bNumDigits) ? aNumDigits : bNumDigits);
	LPA_BCD_digit borrow = 0;

	LPA_BCD_LOG("sumSize:%d\n", sumSize);
	LPA_BCD_allocNumber(pResult, sumSize);

	for (i = 0; i < sumSize; ++i)
	{
		unsigned int j = 0;
		
		LPA_BCD_digitIntermediate aDigit = 0;
		LPA_BCD_digitIntermediate bDigit = 0;
		LPA_BCD_digit sumDigit = 0;
		LPA_BCD_LOG("i:%d\n", i);

		if (i < aNumDigits)
		{
			aDigit = pA->pDigits[i];
		}
		if (i < bNumDigits)
		{
			bDigit = pB->pDigits[i];
		}
		for (j = 0; j < 2; ++j)
		{
			const LPA_BCD_digitIntermediate shift = (LPA_BCD_digitIntermediate)(j << 2);
			const LPA_BCD_digitIntermediate aValue = (aDigit >> shift) & LPA_BCD_DIGIT_MASK;
			const LPA_BCD_digitIntermediate bValue = (bDigit >> shift) & LPA_BCD_DIGIT_MASK;
			LPA_BCD_digitIntermediate sumValue = aValue - bValue - borrow;
			LPA_BCD_LOG("j:%d borrow:%u sumValue:%u aValue:%u bValue:%u\n", j, borrow, sumValue, aValue, bValue);

			if (sumValue < 0)
			{
				borrow = 1;
				sumValue += 10;
				LPA_BCD_LOG("j:%d 2 borrow:%u sumValue:%u aValue:%u bValue:%u\n", j, borrow, sumValue, aValue, bValue);
			}
			else
			{
				borrow = 0;
			}
			sumDigit |= (LPA_BCD_digit)(sumValue << shift);
		}
		pResult->pDigits[i] = sumDigit;
	}
	pResult->negative = 0;
	/* if borrow is set then this is a -ve number */
	if (borrow == 1)
	{
		LPA_BCD_invert(pResult);
	}
}

static void LPA_BCD_multiplyInternal(const LPA_BCD_number* const pA, const LPA_BCD_number* const pB, LPA_BCD_number* const pResult)
{
	LPA_BCD_size i = 0;
	const LPA_BCD_size aNumDigits = pA->numDigits;
	const LPA_BCD_size bNumDigits = pB->numDigits;
	/* multiply length : maximum length */
	const LPA_BCD_size outNumDigits = (aNumDigits + bNumDigits);
	LPA_BCD_digit carry = 0;
	LPA_BCD_size outPosition = 0;

	LPA_BCD_allocNumber(pResult, outNumDigits);

	for (i = 0; i < aNumDigits; ++i)
	{
		unsigned int iN = 0;
		LPA_BCD_digitIntermediate aDigit = 0;
		LPA_BCD_size outPositionI = i * 2;

		if (i < aNumDigits)
		{
			aDigit = pA->pDigits[i];
		}
		for (iN = 0; iN < 2; ++iN)
		{
			const LPA_BCD_digitIntermediate aShift = (LPA_BCD_digitIntermediate)(iN << 2);
			const LPA_BCD_digitIntermediate aValue = (aDigit >> aShift) & LPA_BCD_DIGIT_MASK;
			LPA_BCD_size j = 0;
			outPositionI += iN;
			for (j = 0; j < bNumDigits; ++j)
			{
				unsigned int jN = 0;
				LPA_BCD_digitIntermediate bDigit = 0;
				outPosition = outPositionI;

				if (j < bNumDigits)
				{
					bDigit = pB->pDigits[j];
				}
				outPosition += j * 2;
				for (jN = 0; jN < 2; ++jN)
				{
					int outNibble;
					LPA_BCD_digit result = 0;
					LPA_BCD_digitIntermediate resultValue = 0;
					const LPA_BCD_digitIntermediate bShift = (LPA_BCD_digitIntermediate)(jN << 2);
					const LPA_BCD_digitIntermediate bValue = (bDigit >> bShift) & LPA_BCD_DIGIT_MASK;

					LPA_BCD_digitIntermediate multValue = aValue * bValue + carry;
					LPA_BCD_digitIntermediate units;
					LPA_BCD_digit outNibbleMask;
					int outShift;
					LPA_BCD_size outIndex;

					outPosition += jN;
					outIndex = outPosition >> 1;
					outNibble = outPosition & 0x1;
					outShift = (outNibble << 2);
					outNibbleMask = (LPA_BCD_digit)(0xF0 >> outShift);

					result = pResult->pDigits[outIndex];
					resultValue = (result >> outShift) & LPA_BCD_DIGIT_MASK;
					multValue += resultValue;

					units = multValue % 10;
					carry = (LPA_BCD_digit)multValue / 10;
					LPA_BCD_LOG("i:%d iN:%d j:%d jN:%d a:%d b:%d outIndex:%d outNibble:%d mult:%d units:%d carry:%d result:0x%X\n",
												i, iN, j, jN, aValue, bValue, outIndex, outNibble, multValue, units, carry, pResult->pDigits[outIndex]);

					pResult->pDigits[outIndex] &= outNibbleMask;
					pResult->pDigits[outIndex] |= (LPA_BCD_digit)(units << outShift);

					LPA_BCD_LOG("outNibbleMask:0x%X result:0x%X\n", outNibbleMask, pResult->pDigits[outIndex]);
				}
			}
		}
	}
	if (carry > 0)
	{
		LPA_BCD_size outIndex;
		int outNibble;
		int outShift;
		LPA_BCD_digit outNibbleMask;

		outPosition++;
		outIndex = outPosition >> 1;
		outNibble = outPosition & 0x1;
		outShift = (outNibble << 2);
		outNibbleMask = (LPA_BCD_digit)(0xF0 >> outShift);

		pResult->pDigits[outIndex] &= outNibbleMask;
		pResult->pDigits[outIndex] |= (LPA_BCD_digit)(carry << outShift);
	}
}

/*

Public functions

*/

void LPA_BCD_initNumber(LPA_BCD_number* const pNumber)
{
	pNumber->pDigits = NULL;
	pNumber->numDigits = 0;
	pNumber->negative = 0;
}

void LPA_BCD_freeNumber(LPA_BCD_number* const pNumber)
{
	LPA_freeMem(pNumber->pDigits);
	pNumber->pDigits = NULL;
	pNumber->numDigits = 0;
	pNumber->negative = 0;
}

void LPA_BCD_toDecimalASCII(const LPA_BCD_number* const pNumber, char* const pBuffer, const size_t maxNumChars)
{
	LPA_BCD_size i;
	size_t outIndex = 0;
	const LPA_BCD_size maxLoop = pNumber->numDigits;
	LPA_BCD_digitIntermediate digit = 0;
	size_t maxValidChar = 0;
	int validNumber = 0;

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
			const LPA_BCD_digitIntermediate shift = (LPA_BCD_digitIntermediate)(j << 2);
			const LPA_BCD_digitIntermediate value = (digit >> shift) & LPA_BCD_DIGIT_MASK;
			LPA_BCD_LOG("digit:0x%X shift:%d value:%u\n", digit, shift, value);
			pBuffer[outIndex] = (char)('0' + value);
			if (value > 0)
			{
				maxValidChar = outIndex;
				validNumber = 1;
			}
			++outIndex;
			if (outIndex == maxNumChars)
			{
				--outIndex;
				break;
			}
		}
	}
	if (validNumber == 0)
	{
		pBuffer[0] = '0';
		maxValidChar = 0;
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
void LPA_BCD_fromDecimalASCII(LPA_BCD_number* const pNumber, const char* const value)
{
	const char* pEnd;
	const char* pStr = value;
	LPA_BCD_size index = 0;
	LPA_BCD_size nibbleIndex = 0;
	int negative = 0;

	LPA_BCD_initNumber(pNumber);
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
			LPA_BCD_freeNumber(pNumber);
			return;
		}
		LPA_BCD_LOG("c:%c index:%d nibbleIndex:%d digit:%u\n", c, index, nibbleIndex, digit);
		if (nibbleIndex == 0)
		{
			LPA_BCD_extendNumber(pNumber, 1);
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

void LPA_BCD_fromInt32(LPA_BCD_number* const pNumber, LPA_int32 value)
{
	LPA_BCD_fromInt64(pNumber, value);
}

void LPA_BCD_fromInt64(LPA_BCD_number* const pNumber, LPA_int64 value)
{
	if (value < 0)
	{
		LPA_int64 absValue = -value;
		LPA_BCD_fromUint64(pNumber, (LPA_uint64)absValue);
		pNumber->negative = 1;
	}
	else
	{
		LPA_BCD_fromUint64(pNumber, (LPA_uint64)value);
	}
}

void LPA_BCD_fromUint32(LPA_BCD_number* const pNumber, LPA_uint32 value)
{
	LPA_BCD_fromUint64(pNumber, value);
}

void LPA_BCD_fromUint64(LPA_BCD_number* const pNumber, LPA_uint64 value)
{
	LPA_uint64 workingValue = value;
	LPA_BCD_size index = 0;
	LPA_BCD_size nibbleIndex = 0;

	LPA_BCD_initNumber(pNumber);

	do
	{
		LPA_BCD_digitIntermediate digit = (LPA_BCD_digitIntermediate)(workingValue % 10);
		LPA_BCD_LOG("workingValue:%lu index:%d nibbleIndex:%d digit:%u\n", workingValue, index, nibbleIndex, digit);
		if (nibbleIndex == 0)
		{
			LPA_BCD_extendNumber(pNumber, 1);
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
	while (workingValue != 0);
}

void LPA_BCD_add(const LPA_BCD_number* const pA, const LPA_BCD_number* const pB, LPA_BCD_number* const pResult)
{
	if (pA == NULL)
	{
		return;
	}
	if (pB == NULL)
	{
		return;
	}
	if (pResult == NULL)
	{
		return;
	}
	if (pA->negative)
	{
		if (pB->negative)
		{
			LPA_BCD_addInternal(pA, pB, pResult);
			pResult->negative = 1;
			return;
		}
		else
		{
			LPA_BCD_subtractInternal(pB, pA, pResult);
			return;
		}
	}
	else if (pB->negative)
	{
		LPA_BCD_subtractInternal(pA, pB, pResult);
		return;
	}
	LPA_BCD_addInternal(pA, pB, pResult);
}

void LPA_BCD_subtract(const LPA_BCD_number* const pA, const LPA_BCD_number* const pB, LPA_BCD_number* const pResult)
{
	if (pA == NULL)
	{
		return;
	}
	if (pB == NULL)
	{
		return;
	}
	if (pResult == NULL)
	{
		return;
	}
	if (pA->negative)
	{
		if (pB->negative)
		{
			LPA_BCD_subtractInternal(pB, pA, pResult);
			return;
		}
		else
		{
			LPA_BCD_addInternal(pA, pB, pResult);
			pResult->negative = 1;
			return;
		}
	}
	else if (pB->negative)
	{
		LPA_BCD_addInternal(pA, pB, pResult);
		return;
	}
	LPA_BCD_subtractInternal(pA, pB, pResult);
}

void LPA_BCD_multiply(const LPA_BCD_number* const pA, const LPA_BCD_number* const pB, LPA_BCD_number* const pResult)
{
	if (pA == NULL)
	{
		return;
	}
	if (pB == NULL)
	{
		return;
	}
	if (pResult == NULL)
	{
		return;
	}
	if (pA->negative)
	{
		if (pB->negative)
		{
			LPA_BCD_multiplyInternal(pA, pB, pResult);
			return;
		}
		else
		{
			LPA_BCD_multiplyInternal(pA, pB, pResult);
			pResult->negative = 1;
			return;
		}
	}
	else if (pB->negative)
	{
		LPA_BCD_multiplyInternal(pA, pB, pResult);
		pResult->negative = 1;
		return;
	}
	LPA_BCD_multiplyInternal(pA, pB, pResult);
}

