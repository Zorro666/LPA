/*

Binary coded decimal large precision arithmetic functions

*/

#include <string.h>
#include <stdio.h>

#include "lpa_bcd.h"

#define LPA_BCD_DIGIT_MASK (0xF)

#define LPA_BCD_DEBUG (0)
#define CHAR_BUFFER_SIZE (1024)

#define LPA_BCD_DEBUG_ADD (0)
#define LPA_BCD_DEBUG_SUBTRACT (0)
#define LPA_BCD_DEBUG_MULTIPLY (0)
#define LPA_BCD_DEBUG_DIVIDE (0)
#define LPA_BCD_DEBUG_LENGTH (0)
#define LPA_BCD_DEBUG_INVERT (0)
#define LPA_BCD_DEBUG_TODECIMAL (0)
#define LPA_BCD_DEBUG_FROMDECIMAL (0)
#define LPA_BCD_DEBUG_FROMUINT64 (0)

#if LPA_BCD_DEBUG
#define LPA_BCD_LOG LPA_LOG
#else
#define LPA_BCD_LOG if (0) LPA_LOG
#endif 

#if LPA_BCD_DEBUG_ADD && LPA_BCD_DEBUG
#define LPA_BCD_LOG_ADD LPA_LOG("ADD:"); LPA_LOG
#else
#define LPA_BCD_LOG_ADD if (0) LPA_LOG
#endif

#if LPA_BCD_DEBUG_SUBTRACT && LPA_BCD_DEBUG
#define LPA_BCD_LOG_SUBTRACT LPA_LOG("SUBTRACT:"); LPA_LOG
#else
#define LPA_BCD_LOG_SUBTRACT if (0) LPA_LOG
#endif

#if LPA_BCD_DEBUG_MULTIPLY && LPA_BCD_DEBUG
#define LPA_BCD_LOG_MULTIPLY LPA_LOG("MULTIPLY:"); LPA_LOG
#else
#define LPA_BCD_LOG_MULTIPLY if (0) LPA_LOG
#endif

#if LPA_BCD_DEBUG_DIVIDE && LPA_BCD_DEBUG
#define LPA_BCD_LOG_DIVIDE LPA_LOG("DIVIDE:"); LPA_LOG
#else
#define LPA_BCD_LOG_DIVIDE if (0) LPA_LOG
#endif

#if LPA_BCD_DEBUG_LENGTH && LPA_BCD_DEBUG
#define LPA_BCD_LOG_LENGTH LPA_LOG("LENGTH:"); LPA_LOG
#else
#define LPA_BCD_LOG_LENGTH if (0) LPA_LOG
#endif

#if LPA_BCD_DEBUG_INVERT && LPA_BCD_DEBUG
#define LPA_BCD_LOG_INVERT LPA_LOG("INVERT:"); LPA_LOG
#else
#define LPA_BCD_LOG_INVERT if (0) LPA_LOG
#endif

#if LPA_BCD_DEBUG_TODECIMAL && LPA_BCD_DEBUG
#define LPA_BCD_LOG_TODECIMAL LPA_LOG("TODECIMAL:"); LPA_LOG
#else
#define LPA_BCD_LOG_TODECIMAL if (0) LPA_LOG
#endif

#if LPA_BCD_DEBUG_FROMDECIMAL && LPA_BCD_DEBUG
#define LPA_BCD_LOG_FROMDECIMAL LPA_LOG("FROMDECIMAL:"); LPA_LOG
#else
#define LPA_BCD_LOG_FROMDECIMAL if (0) LPA_LOG
#endif

#if LPA_BCD_DEBUG_FROMUINT64 && LPA_BCD_DEBUG
#define LPA_BCD_LOG_FROMUINT64 LPA_LOG("FROMUINT64:"); LPA_LOG
#else
#define LPA_BCD_LOG_FROMUINT64 if (0) LPA_LOG
#endif

typedef LPA_int32 LPA_BCD_digitIntermediate;

/*

Private functions

*/

static void LPA_BCD_copyNumber(LPA_BCD_number* pDst, const LPA_BCD_number* const pSrc)
{
	const LPA_BCD_size numDigits = pSrc->numDigits;
	const LPA_BCD_size memorySize = numDigits * sizeof(LPA_BCD_digit);
	if (pDst->pDigits != NULL)
	{
		LPA_freeMem(pDst->pDigits);
		pDst->pDigits = NULL;
	}
	if (memorySize > 0)
	{
		pDst->pDigits = LPA_allocMem(memorySize);
	}
	memcpy(pDst->pDigits, pSrc->pDigits, memorySize);
	pDst->numDigits = numDigits;
	pDst->negative = pSrc->negative;
}

static LPA_BCD_size LPA_BCD_length(const LPA_BCD_number* const pNumber)
{
	LPA_BCD_size index = 0;
	LPA_BCD_size length = 0;
	LPA_BCD_size numDigits = pNumber->numDigits;
	LPA_BCD_size i = 0;

	for (i = 0; i < numDigits; ++i)
	{
		const LPA_BCD_digitIntermediate digit = pNumber->pDigits[i];
		unsigned int j = 0;
		for (j = 0; j < 2; ++j)
		{
			const LPA_BCD_digitIntermediate shift = (LPA_BCD_digitIntermediate)(j << 2);
			const LPA_BCD_digitIntermediate value = (LPA_BCD_digitIntermediate)(digit >> shift) & LPA_BCD_DIGIT_MASK;
			++index;
			LPA_BCD_LOG_LENGTH("digit[%d:%d]:0x%X shift:%d value:%d index:%d\n", i, j, digit, shift, value, index);
			if (value != 0)
			{
				if (index > length)
				{
					length = index;
				}
			}
		}
	}
	return length;
}

static LPA_BCD_digit LPA_BCD_getDigit(const LPA_BCD_number* const pNumber, const LPA_BCD_size index)
{
	const LPA_BCD_size digitIndex = (index >> 1);

	if (digitIndex < pNumber->numDigits)
	{
		const LPA_BCD_digit digit = pNumber->pDigits[digitIndex];
		const LPA_BCD_size nibbleIndex = (index & 0x1);
		const LPA_BCD_digit shift = (LPA_BCD_digit)(nibbleIndex << 2);
		const LPA_BCD_digit value = (LPA_BCD_digit)(digit >> shift) & LPA_BCD_DIGIT_MASK;
		return value;
	}

	return 0;
}

static void LPA_BCD_setDigit(const LPA_BCD_number* const pNumber, const LPA_BCD_size index, const LPA_BCD_digit value)
{
	const LPA_BCD_size digitIndex = (index >> 1);

	if (digitIndex < pNumber->numDigits)
	{
		const LPA_BCD_size nibbleIndex = (index & 0x1);
		const LPA_BCD_digit shift = (LPA_BCD_digit)(nibbleIndex << 2);
		const LPA_BCD_digit nibbleMask = (LPA_BCD_digit)(0xF0 >> shift);
		pNumber->pDigits[digitIndex] &= nibbleMask;
		pNumber->pDigits[digitIndex] |= (LPA_BCD_digit)(value << shift);
	}
}

static void LPA_BCD_zeroNumber(LPA_BCD_number* const pNumber)
{
	LPA_freeMem(pNumber->pDigits);
	pNumber->pDigits = NULL;
	pNumber->numDigits = 0;
	pNumber->negative = 0;
}

static void LPA_BCD_correctNegative(LPA_BCD_number* const pResult)
{
	/* Convert -ve 0 -> +ve 0 */
	if (pResult->negative == 1)
	{
		LPA_BCD_size j = pResult->numDigits;
		while (j--)
		{
			if (pResult->pDigits[j] != 0x0)
			{
				return;
			}
		}
		pResult->negative = 0;
	}
}

static void LPA_BCD_invert(LPA_BCD_number* const pNumber)
{
	/* loop over the digits: 10^(n+1) - number */
	LPA_BCD_size i;
	const LPA_BCD_size maxLoop = pNumber->numDigits;
	LPA_BCD_digitIntermediate carry = 1;

	for (i = 0; i < maxLoop; ++i)
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
			LPA_BCD_LOG_INVERT("i:%u j:%u value:%d outValue:%d\n", i, j, value, outValue);
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
		if (pOldDigits != NULL)
		{
			memcpy(pNumber->pDigits, pOldDigits, oldMemorySize);
			LPA_freeMem((void*)pOldDigits);
		}
		memset(pNumber->pDigits+oldNumDigits, 0, newMemorySize-oldMemorySize);
	}
}

static void LPA_BCD_allocNumber(LPA_BCD_number* const pNumber, const LPA_BCD_size newNumDigits)
{
	const size_t newMemorySize = newNumDigits * sizeof(LPA_BCD_digit);
	if (pNumber == NULL)
	{
		LPA_ERROR("LPA_BCD_allocNumber::pNumber is NULL");
		return;
	}
	if (newNumDigits != pNumber->numDigits)
	{
		if (pNumber->pDigits != NULL)
		{
			LPA_freeMem(pNumber->pDigits);
			pNumber->pDigits = NULL;
		}
		if (newMemorySize > 0)
		{
			pNumber->pDigits = LPA_allocMem(newMemorySize);
		}
		pNumber->numDigits = newNumDigits;
	}
	memset(pNumber->pDigits, 0, newMemorySize);
	pNumber->negative = 0;
}

static void LPA_BCD_addInternal(LPA_BCD_number* const pResult, const LPA_BCD_number* const pA, const LPA_BCD_number* const pB)
{
	LPA_BCD_size i = 0;
	const LPA_BCD_size aNumDigits = pA->numDigits;
	const LPA_BCD_size bNumDigits = pB->numDigits;
	/* sum length : extend if needed for the carry */
	const LPA_BCD_size sumSize = ((aNumDigits > bNumDigits) ? aNumDigits : bNumDigits);
	LPA_BCD_digit carry = 0;

	LPA_BCD_LOG_ADD("sumSize:%d\n", sumSize);
	LPA_BCD_allocNumber(pResult, sumSize);

	for (i = 0; i < sumSize; ++i)
	{
		unsigned int j = 0;
		
		LPA_BCD_digitIntermediate aDigit = 0;
		LPA_BCD_digitIntermediate bDigit = 0;
		LPA_BCD_digit sumDigit = 0;
		LPA_BCD_LOG_ADD("i:%d\n", i);

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
			LPA_BCD_LOG_ADD("carry:%u sumValue:%u aValue:%u bValue:%u\n", carry, sumValue, aValue, bValue);

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

static void LPA_BCD_subtractInternal(LPA_BCD_number* const pResult, const LPA_BCD_number* const pA, const LPA_BCD_number* const pB)
{
	LPA_BCD_size i = 0;
	const LPA_BCD_size aNumDigits = pA->numDigits;
	const LPA_BCD_size bNumDigits = pB->numDigits;
	/* sum length : extend if needed for the borrow */
	const LPA_BCD_size sumSize = ((aNumDigits > bNumDigits) ? aNumDigits : bNumDigits);
	LPA_BCD_digit borrow = 0;

	LPA_BCD_LOG_SUBTRACT("sumSize:%d\n", sumSize);
	LPA_BCD_allocNumber(pResult, sumSize);

	for (i = 0; i < sumSize; ++i)
	{
		unsigned int j = 0;
		
		LPA_BCD_digitIntermediate aDigit = 0;
		LPA_BCD_digitIntermediate bDigit = 0;
		LPA_BCD_digit sumDigit = 0;
		LPA_BCD_LOG_SUBTRACT("i:%d\n", i);

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
			LPA_BCD_LOG_SUBTRACT("j:%d borrow:%u sumValue:%u aValue:%u bValue:%u\n", j, borrow, sumValue, aValue, bValue);

			if (sumValue < 0)
			{
				borrow = 1;
				sumValue += 10;
				LPA_BCD_LOG_SUBTRACT("j:%d 2 borrow:%u sumValue:%u aValue:%u bValue:%u\n", j, borrow, sumValue, aValue, bValue);
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

static void LPA_BCD_multiplyInternal(LPA_BCD_number* const pResult, const LPA_BCD_number* const pA, const LPA_BCD_number* const pB)
{
	LPA_BCD_size i = 0;
	const LPA_BCD_size aNumDigits = pA->numDigits;
	const LPA_BCD_size bNumDigits = pB->numDigits;
	/* multiply length : maximum length */
	const LPA_BCD_size outNumDigits = (aNumDigits + bNumDigits);

	if (aNumDigits == 0)
	{
		LPA_BCD_zeroNumber(pResult);
		return;
	}
	if (bNumDigits == 0)
	{
		LPA_BCD_zeroNumber(pResult);
		return;
	}

	LPA_BCD_allocNumber(pResult, outNumDigits);

	for (i = 0; i < aNumDigits; ++i)
	{
		LPA_BCD_size outPosition = 0;
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
			LPA_BCD_digitIntermediate carry = 0;
			LPA_BCD_digit outNibbleMask;
			int outShift;
			int outNibble;
			LPA_BCD_size outIndex;
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
					LPA_BCD_digit result = 0;
					LPA_BCD_digitIntermediate resultValue = 0;
					const LPA_BCD_digitIntermediate bShift = (LPA_BCD_digitIntermediate)(jN << 2);
					const LPA_BCD_digitIntermediate bValue = (bDigit >> bShift) & LPA_BCD_DIGIT_MASK;

					LPA_BCD_digitIntermediate multValue = aValue * bValue + carry;
					LPA_BCD_digitIntermediate units;

					outPosition += jN;
					outIndex = outPosition >> 1;
					outNibble = outPosition & 0x1;
					outShift = (outNibble << 2);
					outNibbleMask = (LPA_BCD_digit)(0xF0 >> outShift);

					result = pResult->pDigits[outIndex];
					resultValue = (result >> outShift) & LPA_BCD_DIGIT_MASK;
					multValue += resultValue;

					units = multValue % 10;
					carry = multValue / 10;
					LPA_BCD_LOG_MULTIPLY("i:%d iN:%d j:%d jN:%d a:%d b:%d rV:%d outIndex:%d outNibble:%d mult:%d units:%d carry:%d result:0x%X\n",
												i, iN, j, jN, aValue, bValue, resultValue, 
												outIndex, outNibble, multValue, units, carry, pResult->pDigits[outIndex]);

					pResult->pDigits[outIndex] &= outNibbleMask;
					pResult->pDigits[outIndex] |= (LPA_BCD_digit)(units << outShift);

					LPA_BCD_LOG_MULTIPLY("outNibbleMask:0x%X result:0x%X\n", outNibbleMask, pResult->pDigits[outIndex]);
				}
			}
			++outPosition;
			outIndex = outPosition >> 1;
			outNibble = outPosition & 0x1;
			outShift = (outNibble << 2);
			outNibbleMask = (LPA_BCD_digit)(0xF0 >> outShift);

			pResult->pDigits[outIndex] &= outNibbleMask;
			pResult->pDigits[outIndex] |= (LPA_BCD_digit)(carry << outShift);
		}
	}
}

static void LPA_BCD_singleMultiply(LPA_BCD_number* const pResult, const LPA_BCD_number* const pA, const LPA_BCD_digit b)
{
	LPA_BCD_size i = 0;
	const LPA_BCD_size aNumDigits = pA->numDigits;
	const LPA_BCD_size bNumDigits = 1;
	/* multiply length : maximum length */
	const LPA_BCD_size outNumDigits = (aNumDigits + bNumDigits);
	const	LPA_BCD_digitIntermediate bValue = (LPA_BCD_digitIntermediate)b;
	LPA_BCD_size outPosition = 0;

	if (aNumDigits == 0)
	{
		LPA_BCD_zeroNumber(pResult);
		return;
	}

	LPA_BCD_allocNumber(pResult, outNumDigits);

	for (i = 0; i < aNumDigits; ++i)
	{
		unsigned int iN = 0;
		LPA_BCD_digitIntermediate aDigit = 0;

		if (i < aNumDigits)
		{
			aDigit = pA->pDigits[i];
		}
		for (iN = 0; iN < 2; ++iN)
		{
			const LPA_BCD_digitIntermediate aShift = (LPA_BCD_digitIntermediate)(iN << 2);
			const LPA_BCD_digitIntermediate aValue = (aDigit >> aShift) & LPA_BCD_DIGIT_MASK;
			LPA_BCD_digitIntermediate carry = 0;
			LPA_BCD_digit outNibbleMask;
			int outShift;
			int outNibble;
			LPA_BCD_size outIndex;
			LPA_BCD_digit result = 0;
			LPA_BCD_digitIntermediate resultValue = 0;
			LPA_BCD_digitIntermediate multValue = aValue * bValue + carry;
			LPA_BCD_digitIntermediate units;

			outIndex = outPosition >> 1;
			outNibble = outPosition & 0x1;
			outShift = (outNibble << 2);
			outNibbleMask = (LPA_BCD_digit)(0xF0 >> outShift);

			result = pResult->pDigits[outIndex];
			resultValue = (result >> outShift) & LPA_BCD_DIGIT_MASK;
			multValue += resultValue;

			units = multValue % 10;
			carry = multValue / 10;

			pResult->pDigits[outIndex] &= outNibbleMask;
			pResult->pDigits[outIndex] |= (LPA_BCD_digit)(units << outShift);

			++outPosition;
			outIndex = outPosition >> 1;
			outNibble = outPosition & 0x1;
			outShift = (outNibble << 2);
			outNibbleMask = (LPA_BCD_digit)(0xF0 >> outShift);

			pResult->pDigits[outIndex] &= outNibbleMask;
			pResult->pDigits[outIndex] |= (LPA_BCD_digit)(carry << outShift);
		}
	}
}

static void LPA_BCD_singleDivide(LPA_BCD_number* const pQuotient, LPA_BCD_number* const pRemainder, 
																	const LPA_BCD_number* const pA, const LPA_BCD_digit b)
{
	LPA_BCD_size jLoop;
	LPA_BCD_size aSize;
	LPA_BCD_size outIndex;
	LPA_BCD_digitIntermediate q = 0;
	LPA_BCD_digitIntermediate aPartial = 0;
	LPA_BCD_number temp1;
	LPA_BCD_number bWork;
#if LPA_BCD_DEBUG_DIVIDE
	char outBuffer[CHAR_BUFFER_SIZE];
#endif

	aSize = LPA_BCD_length(pA);
	LPA_BCD_initNumber(pQuotient);
	LPA_BCD_allocNumber(pQuotient, aSize);
	outIndex = aSize - 1;
#if LPA_BCD_DEBUG_DIVIDE
	LPA_BCD_toDecimalASCII(outBuffer, CHAR_BUFFER_SIZE, pA);
	LPA_BCD_LOG_DIVIDE("aSize:%d %s\n", aSize, outBuffer);
#endif
	for (jLoop = 0; jLoop < aSize; ++jLoop, --outIndex)
	{
		LPA_BCD_size j = outIndex;
		LPA_BCD_digit aJ = LPA_BCD_getDigit(pA, j);
		aPartial = aPartial * 10 + aJ;
		q = aPartial / b;
		LPA_BCD_setDigit(pQuotient, outIndex, (LPA_BCD_digit)q);
		aPartial -= b * q;
	}
#if LPA_BCD_DEBUG_DIVIDE
	LPA_BCD_toDecimalASCII(outBuffer, CHAR_BUFFER_SIZE, pQuotient);
	LPA_BCD_LOG_DIVIDE("Quotient:%s\n", outBuffer);
#endif
	pQuotient->negative = 0;
	LPA_BCD_fromInt32(&bWork, b);
	LPA_BCD_initNumber(&temp1);
	LPA_BCD_multiply(&temp1, pQuotient, &bWork);
#if LPA_BCD_DEBUG_DIVIDE
	LPA_BCD_toDecimalASCII(outBuffer, CHAR_BUFFER_SIZE, &temp1);
	LPA_BCD_LOG_DIVIDE("temp1:%s\n", outBuffer);
#endif
	LPA_BCD_subtract(pRemainder, pA, &temp1);
#if LPA_BCD_DEBUG_DIVIDE
	LPA_BCD_toDecimalASCII(outBuffer, CHAR_BUFFER_SIZE, pRemainder);
	LPA_BCD_LOG_DIVIDE("Remainder:%s\n", outBuffer);
#endif

	LPA_BCD_freeNumber(&temp1);
	LPA_BCD_freeNumber(&bWork);
	pRemainder->negative = 0;
}

/* This function isn't optimised - intended to be used a function to compute reference results for other LPA functions */
/* Packing data into nibbles makes this function more complex because of use of getDigit, setDigit */
static void LPA_BCD_divideInternal(LPA_BCD_number* const pQuotient, LPA_BCD_number* const pRemainder,
																		const LPA_BCD_number* const pA, const LPA_BCD_number* const pB)
{
	/* Q = A/B */
	/* R = A mod B */
	LPA_BCD_size aSize;
	LPA_BCD_size bSize;
	LPA_BCD_size jLoop;
	LPA_BCD_size i;
	LPA_BCD_size m;
	LPA_BCD_size mAllocSize;
	LPA_BCD_size n;
	LPA_BCD_size aWorkLen;
	LPA_BCD_digitIntermediate B1;
	LPA_BCD_number aWork;
	LPA_BCD_number bWork;
	LPA_BCD_number temp1;
	LPA_BCD_number temp2;
	LPA_BCD_number temp3;
	LPA_BCD_digit bWorkNm1;
	LPA_BCD_digit bWorkNm2;
	LPA_int64 spD;
#if LPA_BCD_DEBUG_DIVIDE
	char outBuffer[CHAR_BUFFER_SIZE];
#endif

	aSize = LPA_BCD_length(pA);
	bSize = LPA_BCD_length(pB);

	/* n = B_size */
	n = bSize;

	LPA_BCD_LOG_DIVIDE("aSize:%d bSize:%d\n", aSize, bSize);
	if (n == 0)
	{
		/* divide by zero */
		LPA_BCD_zeroNumber(pQuotient);
		LPA_BCD_zeroNumber(pRemainder);
		return;
	}
	if (n == 1)
	{
		/* single digit divide */
		const LPA_BCD_digit b = LPA_BCD_getDigit(pB, 0);
		LPA_BCD_singleDivide(pQuotient, pRemainder, pA, b);
		return;
	}

	if (bSize > aSize)
	{
		/* quotient = zero */
		LPA_BCD_zeroNumber(pQuotient);
		/* remainder = A */
		LPA_BCD_copyNumber(pRemainder, pA);
		return;
	}

	LPA_BCD_initNumber(&aWork);
	LPA_BCD_initNumber(&bWork);
	LPA_BCD_initNumber(&temp1);
	LPA_BCD_initNumber(&temp2);
	LPA_BCD_initNumber(&temp3);
	LPA_BCD_initNumber(pQuotient);
	LPA_BCD_initNumber(pRemainder);

	/* m = A_size - n */
	m = aSize - n;
	mAllocSize = m + 1;
	LPA_BCD_allocNumber(pQuotient, mAllocSize);

	LPA_BCD_LOG_DIVIDE("m:%d n:%d\n", m, n);

	/* D1. Normalize: D = <base> / (B[n-1]+1) */
	/* D can be arbitrary as long as B[1]*D >= base/2 e.g. D can made into a power of 2 for power of 2 bases */
	B1 = LPA_BCD_getDigit(pB, n-1);
	spD = 10 / (B1 + 1);
	LPA_BCD_fromInt64(&temp1, spD);

	LPA_BCD_LOG_DIVIDE("B1:%d spD:%ld\n", B1, spD);

	/* Awork = A * D : must always add a leading digit */
	LPA_BCD_multiply(&aWork, pA, &temp1);
	aWork.negative = 0;
	aWorkLen = LPA_BCD_length(&aWork);

#if LPA_BCD_DEBUG_DIVIDE
	LPA_BCD_toDecimalASCII(outBuffer, CHAR_BUFFER_SIZE, &aWork);
	LPA_BCD_LOG_DIVIDE("aWork:%s aSize:%d aWorkLen:%d\n", outBuffer, aSize, aWorkLen);
#endif
	if (LPA_BCD_length(pA) == aWorkLen)
	{
		LPA_BCD_LOG_DIVIDE("extend\n");
		LPA_BCD_extendNumber(&aWork, 1);
		aWorkLen++;
	}
	/* Bwork = B * D */
	LPA_BCD_multiply(&bWork, pB, &temp1);
	bWork.negative = 0;
#if LPA_BCD_DEBUG_DIVIDE
	LPA_BCD_toDecimalASCII(outBuffer, CHAR_BUFFER_SIZE, &bWork);
	LPA_BCD_LOG_DIVIDE("bWork:%s\n", outBuffer);
#endif

	LPA_BCD_freeNumber(&temp1);

	bWorkNm1 = LPA_BCD_getDigit(&bWork, n-1);
	bWorkNm2 = LPA_BCD_getDigit(&bWork, n-2);

	LPA_BCD_allocNumber(&temp2, (n+1));

	jLoop = 0;
	do
	{
		LPA_BCD_size j = aWorkLen - jLoop - 1;
		LPA_BCD_digitIntermediate qUnit = 9;
		LPA_BCD_digit aWorkJ = LPA_BCD_getDigit(&aWork, j);
		LPA_BCD_digit aWorkJm1 = LPA_BCD_getDigit(&aWork, j-1);
		LPA_BCD_digit aWorkJm2 = LPA_BCD_getDigit(&aWork, j-2);

		LPA_BCD_LOG_DIVIDE("jLoop:%u j:%d aWorkJ:%d bWorkNm1:%d\n", jLoop, j, aWorkJ, bWorkNm1);
		/* D3. Calculate Qunit: if Awork[j] == Bwork[1], Qunit = base - 1 else Qunit = (Awork[j] * base + Awork[j+1]) / Bwork[1] */
		if (aWorkJ != bWorkNm1)
		{
			qUnit = (aWorkJ * 10 + aWorkJm1) / bWorkNm1;
			LPA_BCD_LOG_DIVIDE("qUnit: %d jLoop:%u aWorkJm1:%d\n", qUnit, jLoop, aWorkJm1);
		}
		/* if (Bwork[2] * qUnit > (Awork[j] * 10 + Awork[j+1] - qUnit * Bwork[1])* 10 + Awork[j+2] qUnit -=1 and test again */
		if ((bWorkNm2 * qUnit) >  ((aWorkJ * 10 + aWorkJm1) - qUnit * bWorkNm1) * 10 + aWorkJm2)
		{
			--qUnit;
			LPA_BCD_LOG_DIVIDE("qUnit: %d one too big\n", qUnit);
			if ((bWorkNm2 * qUnit) >  ((aWorkJ * 10 + aWorkJm1) - qUnit * bWorkNm1) * 10 + aWorkJm2)
			{
				--qUnit;
				LPA_BCD_LOG_DIVIDE("qUnit: %d two too big\n", qUnit);
			}
		}

		/* if (Bwork[2] * qUnit > Awork[j] * 10 + Awork[j+1] - qUnit * Bwork[1] qUnit -=1 */
		/* D4. Multiply & Subtract: Awork[j...j+n] = A[j...j+n] - Qunit * Bwork, save the borrow flag if Awork is -ve */
		LPA_BCD_singleMultiply(&temp1, &bWork, (LPA_BCD_digit)qUnit);
#if LPA_BCD_DEBUG_DIVIDE
		LPA_BCD_LOG_DIVIDE("qUnit: %d n:%d j:%d jLoop:%d n-j:%d\n", qUnit, n, j, jLoop, n-j);
		LPA_BCD_toDecimalASCII(outBuffer, CHAR_BUFFER_SIZE, &temp1);
		LPA_BCD_LOG_DIVIDE("temp1: %s\n", outBuffer);
#endif

		LPA_BCD_LOG_DIVIDE("numTempDigits:%d n:%d jLoop:%d\n", n+1, n, jLoop);
		if (aSize-jLoop <= aSize)
		{
			for (i = 0; i < (n+1); ++i)
			{
				LPA_BCD_size dstDigit = n-i;
				LPA_BCD_size srcDigit = (aSize-1) - i - jLoop + 1;
				LPA_BCD_digit src = LPA_BCD_getDigit(&aWork, srcDigit);
				LPA_BCD_LOG_DIVIDE("src[%d] temp2[%d]: %d\n", srcDigit, dstDigit, src);
				LPA_BCD_setDigit(&temp2, dstDigit, src);
			}
		}
		else
		{
			/* This an error */
			LPA_ERROR("BCD divide: not enough digits for remainder\n");
			LPA_BCD_zeroNumber(pQuotient);
			LPA_BCD_zeroNumber(pRemainder);
			return;
		}
#if LPA_BCD_DEBUG_DIVIDE
		LPA_BCD_toDecimalASCII(outBuffer, CHAR_BUFFER_SIZE, &temp2);
		LPA_BCD_LOG_DIVIDE("temp2: %s\n", outBuffer);
#endif

		LPA_BCD_subtract(&temp3, &temp2, &temp1);
#if LPA_BCD_DEBUG_DIVIDE
		LPA_BCD_toDecimalASCII(outBuffer, CHAR_BUFFER_SIZE, &temp3);
		LPA_BCD_LOG_DIVIDE("temp3: %s\n", outBuffer);
#endif
		/* D5. if borrow flag then Q[j] -= 1 & invert Awork,  Awork = Awork + Bwork */
		if (temp3.negative)
		{
			--qUnit;
			LPA_BCD_add(&temp2, &temp3, &bWork);
			LPA_BCD_copyNumber(&temp3, &temp2);
#if LPA_BCD_DEBUG_DIVIDE
			LPA_BCD_toDecimalASCII(outBuffer, CHAR_BUFFER_SIZE, &temp3);
			LPA_BCD_LOG_DIVIDE("negative temp3: %s\n", outBuffer);
#endif
		}

		for (i = 0; i < (n+1); ++i)
		{
			LPA_BCD_size srcDigit = n-i;
			LPA_BCD_size dstDigit = (aSize-1) - i - jLoop + 1;
			LPA_BCD_digit src = LPA_BCD_getDigit(&temp3, srcDigit);
			LPA_BCD_LOG_DIVIDE("src[%d] aWork[%d]: %d\n", srcDigit, dstDigit, src);
			LPA_BCD_setDigit(&aWork, dstDigit, src);
		}
#if LPA_BCD_DEBUG_DIVIDE
		LPA_BCD_toDecimalASCII(outBuffer, CHAR_BUFFER_SIZE, &aWork);
		LPA_BCD_LOG_DIVIDE("aWork: %s\n", outBuffer);
#endif

		/* D6. Set Q[j] = Qunit */
		LPA_BCD_LOG_DIVIDE("pQuotient[%d]: %d\n", mAllocSize - 1 - jLoop, qUnit);
		LPA_BCD_setDigit(pQuotient, mAllocSize - 1 - jLoop, (LPA_BCD_digit)qUnit);
		++jLoop;
	}
	while (jLoop <= m);
	/* D8. Q = u/v, R = Awork[m+1..m+n]/D */
	LPA_BCD_singleDivide(pRemainder, &temp2, &aWork, (LPA_BCD_digit)spD);

	LPA_BCD_freeNumber(&aWork);
	LPA_BCD_freeNumber(&bWork);
	LPA_BCD_freeNumber(&temp1);
	LPA_BCD_freeNumber(&temp2);
	LPA_BCD_freeNumber(&temp3);
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
	LPA_BCD_zeroNumber(pNumber);
}

void LPA_BCD_toHexadecimalASCII(char* const pBuffer, const size_t maxNumChars, const LPA_BCD_number* const pNumber)
{
	LPA_BCD_size i;
	size_t outIndex = 0;
	const LPA_BCD_size maxLoop = pNumber->numDigits;
	size_t maxValidChar = 0;
	int validNumber = 0;
	LPA_BCD_number number;
	LPA_BCD_number remainder;
	LPA_BCD_size len = 0;

	if (maxLoop == 0)
	{
		pBuffer[0] = '0';
		pBuffer[1] = '\0';
		return;
	}

	LPA_BCD_initNumber(&number);
	LPA_BCD_copyNumber(&number, pNumber);
	number.negative = 0;

	do
	{
		LPA_BCD_number work;
		char c = '*';
		LPA_BCD_digitIntermediate digit = 0;
		LPA_BCD_digitIntermediate value = 0;
		LPA_BCD_initNumber(&work);
		LPA_BCD_initNumber(&remainder);
		LPA_BCD_singleDivide(&work, &remainder, &number, 16);
		if (remainder.numDigits > 0)
		{
			digit = remainder.pDigits[0];
		}
		value = ((digit >> 0) & LPA_BCD_DIGIT_MASK);
		value += 10 * ((digit >> 4) & LPA_BCD_DIGIT_MASK);

		LPA_BCD_copyNumber(&number, &work);
		LPA_BCD_freeNumber(&work);
		LPA_BCD_freeNumber(&remainder);

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
			validNumber = 1;
		}
		++outIndex;
		if (outIndex == maxNumChars)
		{
			--outIndex;
			break;
		}

		len = LPA_BCD_length(&number);
	} while (len > 0);

	LPA_BCD_freeNumber(&number);
	LPA_BCD_freeNumber(&remainder);

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
		++outIndex;
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

void LPA_BCD_toDecimalASCII(char* const pBuffer, const size_t maxNumChars, const LPA_BCD_number* const pNumber)
{
	LPA_BCD_size i;
	size_t outIndex = 0;
	const LPA_BCD_size maxLoop = pNumber->numDigits;
	LPA_BCD_digitIntermediate digit = 0;
	size_t maxValidChar = 0;
	int validNumber = 0;

	if (maxLoop == 0)
	{
		pBuffer[0] = '0';
		pBuffer[1] = '\0';
		return;
	}
	LPA_BCD_LOG_TODECIMAL("maxLoop:%u\n", maxLoop);
	for (i = 0; i < maxLoop; ++i)
	{
		unsigned int j = 0;
		digit = pNumber->pDigits[i];

		for (j = 0; j < 2; ++j)
		{
			const LPA_BCD_digitIntermediate shift = (LPA_BCD_digitIntermediate)(j << 2);
			const LPA_BCD_digitIntermediate value = (digit >> shift) & LPA_BCD_DIGIT_MASK;
			LPA_BCD_LOG_TODECIMAL("digit:0x%X shift:%d value:%u\n", digit, shift, value);
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
		++outIndex;
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

void LPA_BCD_fromHexadecimalASCII(LPA_BCD_number* const pNumber, const char* const value)
{
	const char* pStr = value;
	int negative = 0;
	LPA_BCD_number base;

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
		if ((c == '-') || (c == '+') || (((c >= '0') && (c <= '9')) || ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f'))))
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

	LPA_BCD_fromUint64(&base, 16);
	do
	{
		LPA_BCD_number temp1;
		LPA_BCD_number temp2;
		const char c = *pStr;
		LPA_BCD_digitIntermediate digit = 0;

		if ((c >= '0') && (c <= '9'))
		{
			digit = (LPA_BCD_digit)(c - '0');
		}
		else if ((c >= 'A') && (c <= 'F'))
		{
			digit = (LPA_BCD_digit)(c - 'A' + 0xA);
		}
		else if ((c >= 'a') && (c <= 'f'))
		{
			digit = (LPA_BCD_digit)(c - 'a' + 0xa);
		}
		if (digit > 15)
		{
			LPA_BCD_freeNumber(pNumber);
			return;
		}
		LPA_BCD_fromInt32(&temp2, digit);
		LPA_BCD_initNumber(&temp1);
		LPA_BCD_multiply(&temp1, pNumber, &base);
		LPA_BCD_add(pNumber, &temp1, &temp2);
		LPA_BCD_freeNumber(&temp1);
		LPA_BCD_freeNumber(&temp2);
		++pStr;
	}
	while (*pStr != '\0');
	pNumber->negative = negative;
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
		LPA_BCD_LOG_FROMDECIMAL("c:%c index:%d nibbleIndex:%d digit:%u\n", c, index, nibbleIndex, digit);
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
		LPA_BCD_LOG_FROMUINT64("workingValue:%lu index:%d nibbleIndex:%d digit:%u\n", workingValue, index, nibbleIndex, digit);
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

void LPA_BCD_add(LPA_BCD_number* const pResult, const LPA_BCD_number* const pA, const LPA_BCD_number* const pB)
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
			LPA_BCD_addInternal(pResult, pA, pB);
			pResult->negative = 1;
			LPA_BCD_correctNegative(pResult);
			return;
		}
		else
		{
			LPA_BCD_subtractInternal(pResult, pB, pA);
			return;
		}
	}
	else if (pB->negative)
	{
		LPA_BCD_subtractInternal(pResult, pA, pB);
		return;
	}
	LPA_BCD_addInternal(pResult, pA, pB);
}

void LPA_BCD_subtract(LPA_BCD_number* const pResult, const LPA_BCD_number* const pA, const LPA_BCD_number* const pB)
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
			LPA_BCD_subtractInternal(pResult, pB, pA);
			return;
		}
		else
		{
			LPA_BCD_addInternal(pResult, pA, pB);
			pResult->negative = 1;
			LPA_BCD_correctNegative(pResult);
			return;
		}
	}
	else if (pB->negative)
	{
		LPA_BCD_addInternal(pResult, pA, pB);
		return;
	}
	LPA_BCD_subtractInternal(pResult, pA, pB);
}

void LPA_BCD_multiply(LPA_BCD_number* const pResult, const LPA_BCD_number* const pA, const LPA_BCD_number* const pB)
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
			LPA_BCD_multiplyInternal(pResult, pA, pB);
			return;
		}
		else
		{
			LPA_BCD_multiplyInternal(pResult, pA, pB);
			pResult->negative = 1;
			LPA_BCD_correctNegative(pResult);
			return;
		}
	}
	else if (pB->negative)
	{
		LPA_BCD_multiplyInternal(pResult, pA, pB);
		pResult->negative = 1;
		LPA_BCD_correctNegative(pResult);
		return;
	}
	LPA_BCD_multiplyInternal(pResult, pA, pB);
}

void LPA_BCD_divide(LPA_BCD_number* const pQuotient, LPA_BCD_number* const pRemainder, 
											const LPA_BCD_number* const pA, const LPA_BCD_number* const pB)
{
	if (pA == NULL)
	{
		return;
	}
	if (pB == NULL)
	{
		return;
	}
	if (pQuotient == NULL)
	{
		return;
	}
	if (pRemainder == NULL)
	{
		return;
	}
	if (pA->negative)
	{
		if (pB->negative)
		{
			LPA_BCD_divideInternal(pQuotient, pRemainder, pA, pB);
			pRemainder->negative = 1;
			LPA_BCD_correctNegative(pRemainder);
			return;
		}
		else
		{
			LPA_BCD_divideInternal(pQuotient, pRemainder, pA, pB);
			pQuotient->negative = 1;
			LPA_BCD_correctNegative(pQuotient);
			pRemainder->negative = 1;
			LPA_BCD_correctNegative(pRemainder);
			return;
		}
	}
	else if (pB->negative)
	{
		LPA_BCD_divideInternal(pQuotient, pRemainder, pA, pB);
		pQuotient->negative = 1;
		LPA_BCD_correctNegative(pQuotient);
		return;
	}
	LPA_BCD_divideInternal(pQuotient, pRemainder, pA, pB);
}

