/*

large precision arithmetic functions

*/

#include <string.h>
#include <stdio.h>

#include "lpa_int.h"

#define LPA_INT_DEBUG (0)

#if LPA_INT_DEBUG
#define LPA_INT_LOG LPA_LOG
#else
#define LPA_INT_LOG if (0) LPA_LOG
#endif 

#define LPA_INT_NUM_BITS_PER_CHAR (4)
#define LPA_INT_NUM_BYTES_PER_DIGIT (4)
#define LPA_INT_NUM_BITS_PER_DIGIT (LPA_INT_NUM_BYTES_PER_DIGIT * 8)
#define LPA_INT_NUM_CHARS_PER_DIGIT (LPA_INT_NUM_BITS_PER_DIGIT/LPA_INT_NUM_BITS_PER_CHAR)
#define LPA_INT_DIGIT_MASK (~0u)

typedef LPA_uint64 LPA_INT_digitIntermediate;
/*

Private functions

*/

static void LPA_INT_extendNumber(LPA_INT_number* const pNumber, const LPA_INT_size sizeIncrement)
{
	if (pNumber == NULL)
	{
		LPA_ERROR("LPA_INT_extendNumber::pNumber is NULL");
		return;
	}
	else
	{
		const LPA_INT_digit* const pOldDigits = pNumber->pDigits;
		const LPA_INT_size oldNumDigits = pNumber->numDigits;
		const LPA_INT_size newNumDigits = oldNumDigits + sizeIncrement;
		const size_t oldMemorySize = oldNumDigits * sizeof(LPA_INT_digit);
		const size_t newMemorySize = newNumDigits * sizeof(LPA_INT_digit);

		pNumber->pDigits = LPA_allocMem(newMemorySize);
		pNumber->numDigits = newNumDigits;
		memcpy(pNumber->pDigits, pOldDigits, oldMemorySize);
		LPA_freeMem((void*)pOldDigits);
	}
}

static void LPA_INT_allocNumber(LPA_INT_number* const pNumber, const LPA_INT_size newNumDigits)
{
	if (pNumber == NULL)
	{
		LPA_ERROR("LPA_INT_allocNumber::pNumber is NULL");
		return;
	}
	if (newNumDigits != pNumber->numDigits)
	{
		const size_t newMemorySize = newNumDigits * sizeof(LPA_INT_digit);
		LPA_freeMem(pNumber->pDigits);
		pNumber->pDigits = LPA_allocMem(newMemorySize);
		pNumber->numDigits = newNumDigits;
		memset(pNumber->pDigits, 0, newMemorySize);
	}
}

static void LPA_INT_addInternal(const LPA_INT_number* const pA, const LPA_INT_number* const pB, LPA_INT_number* const pResult)
{
	LPA_INT_size i = 0;
	const LPA_INT_size aNumDigits = pA->numDigits;
	const LPA_INT_size bNumDigits = pB->numDigits;
	/* sum length : extend if needed for the carry */
	const LPA_INT_size sumSize = ((aNumDigits > bNumDigits) ? aNumDigits : bNumDigits);
	LPA_INT_digit carry = 0;

	LPA_INT_LOG("sumSize:%d\n", sumSize);
	LPA_INT_allocNumber(pResult, sumSize);

	for (i = 0; i < sumSize; ++i)
	{
		LPA_INT_digit aDigit = 0;
		LPA_INT_digit bDigit = 0;
		LPA_INT_digit sumDigit = 0;
		LPA_INT_LOG("i:%d\n", i);

		if (i < aNumDigits)
		{
			aDigit = pA->pDigits[i];
		}
		if (i < bNumDigits)
		{
			bDigit = pB->pDigits[i];
		}
		/* digit add e.g. base precision arithmetic */
		{
			const LPA_INT_digit aValue = aDigit;
			const LPA_INT_digit bValue = bDigit;
			LPA_INT_digit sumValue = aValue + carry;
			LPA_INT_LOG("carry:%u sumValue:%X aValue:%X bValue:%X\n", carry, sumValue, aValue, bValue);

			if (sumValue < carry)
			{
				carry = 1;
			}
			else
			{
				carry = 0;
			}
			sumValue += bValue;
			if (sumValue < bValue)
			{
				carry = 1;
			}
			sumDigit = (LPA_INT_digit)sumValue;
		}
		pResult->pDigits[i] = sumDigit;
	}
	/* Extend sum if carry is set */
	if (carry == 1)
	{
		LPA_INT_extendNumber(pResult, 1);
		pResult->pDigits[sumSize] = 0x1;
	}
}

static void LPA_INT_subtractInternal(const LPA_INT_number* const pA, const LPA_INT_number* const pB, LPA_INT_number* const pResult)
{
	LPA_INT_size i = 0;
	const LPA_INT_size aNumDigits = pA->numDigits;
	const LPA_INT_size bNumDigits = pB->numDigits;
	/* sum length : extend if needed for the carry */
	const LPA_INT_size sumSize = ((aNumDigits > bNumDigits) ? aNumDigits : bNumDigits);
	LPA_INT_digit borrow = 0;

	LPA_INT_LOG("sumSize:%d\n", sumSize);
	LPA_INT_allocNumber(pResult, sumSize);

	for (i = 0; i < sumSize; ++i)
	{
		LPA_INT_digit aDigit = 0;
		LPA_INT_digit bDigit = 0;
		LPA_INT_digit sumDigit = 0;
		LPA_INT_LOG("i:%d\n", i);

		if (i < aNumDigits)
		{
			aDigit = pA->pDigits[i];
		}
		if (i < bNumDigits)
		{
			bDigit = pB->pDigits[i];
		}
		/* digit add e.g. base precision arithmetic */
		{
			const LPA_INT_digit aValue = aDigit;
			const LPA_INT_digit bValue = bDigit;
			LPA_INT_digit sumValue = aValue - borrow;
			LPA_INT_LOG("borrow:%u sumValue:%X aValue:%X bValue:%X\n", borrow, sumValue, aValue, bValue);

			if (sumValue > borrow)
			{
				borrow = 1;
			}
			else
			{
				borrow = 0;
			}
			sumValue -= bValue;
			if (sumValue > bValue)
			{
				borrow = 1;
			}
			sumDigit = (LPA_INT_digit)sumValue;
		}
		pResult->pDigits[i] = sumDigit;
	}
	/* Negative if borrow is set */
	if (borrow == 1)
	{
		/* TODO */
	}
}

static void LPA_INT_multiplyInternal(const LPA_INT_number* const pA, const LPA_INT_number* const pB, LPA_INT_number* const pResult)
{
	LPA_INT_size i = 0;
	const LPA_INT_size aNumDigits = pA->numDigits;
	const LPA_INT_size bNumDigits = pB->numDigits;
	/* multiply length : maximum length */
	const LPA_INT_size outNumDigits = (aNumDigits + bNumDigits);
	LPA_INT_digit carry = 0;
	LPA_INT_size outIndex = 0;

	LPA_INT_allocNumber(pResult, outNumDigits);

	for (i = 0; i < aNumDigits; ++i)
	{
		LPA_INT_digitIntermediate aValue = 0;
		LPA_INT_size j = 0;

		if (i < aNumDigits)
		{
			aValue = pA->pDigits[i];
		}
		for (j = 0; j < bNumDigits; ++j)
		{
			LPA_INT_digitIntermediate bValue = 0;
			LPA_INT_digit result = 0;
			LPA_INT_digitIntermediate multValue;
			LPA_INT_digit units;

			if (j < bNumDigits)
			{
				bValue = pB->pDigits[j];
			}

			multValue = aValue * bValue + carry;

			outIndex = i + j;
			result = pResult->pDigits[outIndex];
			LPA_INT_LOG("i:%d j:%d ind:%d a:0x%lX b:0x%lX mult:0x%lX result:0x%X\n", 
					i, j, outIndex, aValue, bValue, multValue, result);
			multValue += result;

			units = (LPA_INT_digit)(multValue & LPA_INT_DIGIT_MASK);
			carry = (LPA_INT_digit)(multValue >> LPA_INT_NUM_BITS_PER_DIGIT);
			pResult->pDigits[outIndex] = units;
			LPA_INT_LOG("ind:%d units:0x%X carry:0x%X\n", outIndex, units, carry);
		}
		++outIndex;
		pResult->pDigits[outIndex] = carry;
	}
}

/*

Public functions

*/

void LPA_INT_initNumber(LPA_INT_number* const pNumber)
{
	pNumber->pDigits = NULL;
	pNumber->numDigits = 0;
}

void LPA_INT_freeNumber(LPA_INT_number* const pNumber)
{
	LPA_freeMem(pNumber->pDigits);
	pNumber->pDigits = NULL;
	pNumber->numDigits = 0;
}


void LPA_INT_toHexadecimalASCII(const LPA_INT_number* const pNumber, char* const pBuffer, const size_t maxNumChars)
{
	LPA_INT_size i;
	size_t outIndex = 0;
	const LPA_INT_size maxLoop = pNumber->numDigits;
	size_t maxValidChar = 0;

	if (maxLoop == 0)
	{
		pBuffer[0] = '\0';
		return;
	}
	LPA_INT_LOG("maxLoop:%u\n", maxLoop);
	for (i = 0; i < maxLoop; ++i)
	{
		int j;
		LPA_INT_digit digit = pNumber->pDigits[i];
		LPA_INT_LOG("digit[%d]:0x%X\n", i, digit);

		for (j = 0; j < LPA_INT_NUM_CHARS_PER_DIGIT; ++j)
		{
			LPA_uint8 value = (LPA_uint8)((digit >> (j * LPA_INT_NUM_BITS_PER_CHAR)) & 0xF);
			char c = '*';
			if (value > 9)
			{
				c = (char)('A' + (value - 10));
			}
			else
			{
				c = (char)('0' + value);
			}
			pBuffer[outIndex] = c;
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
	pBuffer[outIndex] = '\0';

	/* reverse the string to make it correct for printing */
	for (i = 0; i < outIndex/2; ++i)
	{
		char c = pBuffer[outIndex-1-i];
		pBuffer[outIndex-1-i] = pBuffer[i];
		pBuffer[i] = c;
	}
}

/* Hexadecimal ASCII only */
void LPA_INT_fromHexadecimalASCII(LPA_INT_number* const pNumber, const char* const value)
{
	const char* pEnd;
	const char* pStr = value;
	LPA_INT_size index = 0;
	int charIndex = 0;

	LPA_INT_initNumber(pNumber);
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
		if ((c == '-') || (c == '+') || (((c >= '0') && (c <= '9')) || ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f'))))
		{
			break;
		}
		++pStr;
	}

	if (*pStr == '-')
	{
		++pStr;
	}
	else if (*pStr == '+')
	{
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
		LPA_INT_digit digit = 0;
		if ((c >= '0') && (c <= '9'))
		{
			digit = (LPA_INT_digit)(c - '0');
		}
		else if ((c >= 'A') && (c <= 'F'))
		{
			digit = (LPA_INT_digit)(c - 'A' + 0xA);
		}
		else if ((c >= 'a') && (c <= 'f'))
		{
			digit = (LPA_INT_digit)(c - 'a' + 0xa);
		}
		if (digit > 15)
		{
			LPA_freeMem(pNumber->pDigits);
			LPA_INT_initNumber(pNumber);
			return;
		}
		LPA_INT_LOG("c:%c index:%d digit:%u\n", c, index, digit);
		if (charIndex == 0)
		{
			LPA_INT_extendNumber(pNumber, 1);
			pNumber->pDigits[index] = 0;
		}
		digit = digit << (charIndex * LPA_INT_NUM_BITS_PER_CHAR);
		pNumber->pDigits[index] |= digit;
		++charIndex;
		if (charIndex == LPA_INT_NUM_CHARS_PER_DIGIT)
		{
			++index;
			charIndex = 0;
		}
	}
	while (pStr != pEnd);
}

void LPA_INT_fromInt32(LPA_INT_number* const pNumber, LPA_int32 value)
{
	LPA_INT_fromInt64(pNumber, (LPA_uint32)value);
}

void LPA_INT_fromInt64(LPA_INT_number* const pNumber, LPA_int64 value)
{
	LPA_INT_fromUint64(pNumber, (LPA_uint64)value);
}

void LPA_INT_fromUint32(LPA_INT_number* const pNumber, LPA_uint32 value)
{
	LPA_INT_fromUint64(pNumber, (LPA_uint64)value);
}

void LPA_INT_fromUint64(LPA_INT_number* const pNumber, LPA_uint64 value)
{
	const LPA_INT_digit digit0 = value & LPA_INT_DIGIT_MASK;
	const LPA_INT_digit digit1 = (LPA_INT_digit)(value >> LPA_INT_NUM_BITS_PER_DIGIT);
	LPA_INT_size numDigits = (digit1 == 0) ? 1 : 2;

	LPA_INT_initNumber(pNumber);
	LPA_INT_extendNumber(pNumber, numDigits);

	pNumber->pDigits[0] = digit0;
	if (numDigits > 1)
	{
		pNumber->pDigits[1] = digit1;
	}
}

void LPA_INT_add(const LPA_INT_number* const pA, const LPA_INT_number* const pB, LPA_INT_number* const pResult)
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
	LPA_INT_addInternal(pA, pB, pResult);
}

void LPA_INT_subtract(const LPA_INT_number* const pA, const LPA_INT_number* const pB, LPA_INT_number* const pResult)
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
	LPA_INT_subtractInternal(pA, pB, pResult);
}

void LPA_INT_multiply(const LPA_INT_number* const pA, const LPA_INT_number* const pB, LPA_INT_number* const pResult)
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
	LPA_INT_multiplyInternal(pA, pB, pResult);
}

