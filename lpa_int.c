/*

large precision arithmetic functions

*/

#include <string.h>
#include <stdio.h>

#include "lpa_int.h"

#define LPA_INT_DEBUG (0)

#define CHAR_BUFFER_SIZE (1024)

#if LPA_INT_DEBUG
#define LPA_INT_LOG LPA_LOG
#else
#define LPA_INT_LOG if (0) LPA_LOG
#endif 

#define LPA_INT_DEBUG_ADD (0)
#define LPA_INT_DEBUG_SUBTRACT (0)
#define LPA_INT_DEBUG_MULTIPLY (0)
#define LPA_INT_DEBUG_DIVIDE (0)
#define LPA_INT_DEBUG_LENGTH (0)
#define LPA_INT_DEBUG_INVERT (0)
#define LPA_INT_DEBUG_TOHEX (0)
#define LPA_INT_DEBUG_FROMHEX (0)
#define LPA_INT_DEBUG_FROMUINT64 (0)

#if LPA_INT_DEBUG
#define LPA_INT_LOG LPA_LOG
#else
#define LPA_INT_LOG if (0) LPA_LOG
#endif 

#if LPA_INT_DEBUG_ADD && LPA_INT_DEBUG
#define LPA_INT_LOG_ADD LPA_LOG("ADD:"); LPA_LOG
#else
#define LPA_INT_LOG_ADD if (0) LPA_LOG
#endif

#if LPA_INT_DEBUG_SUBTRACT && LPA_INT_DEBUG
#define LPA_INT_LOG_SUBTRACT LPA_LOG("SUBTRACT:"); LPA_LOG
#else
#define LPA_INT_LOG_SUBTRACT if (0) LPA_LOG
#endif

#if LPA_INT_DEBUG_MULTIPLY && LPA_INT_DEBUG
#define LPA_INT_LOG_MULTIPLY LPA_LOG("MULTIPLY:"); LPA_LOG
#else
#define LPA_INT_LOG_MULTIPLY if (0) LPA_LOG
#endif

#if LPA_INT_DEBUG_DIVIDE && LPA_INT_DEBUG
#define LPA_INT_LOG_DIVIDE LPA_LOG("DIVIDE:"); LPA_LOG
#else
#define LPA_INT_LOG_DIVIDE if (0) LPA_LOG
#endif

#if LPA_INT_DEBUG_LENGTH && LPA_INT_DEBUG
#define LPA_INT_LOG_LENGTH LPA_LOG("LENGTH:"); LPA_LOG
#else
#define LPA_INT_LOG_LENGTH if (0) LPA_LOG
#endif

#if LPA_INT_DEBUG_INVERT && LPA_INT_DEBUG
#define LPA_INT_LOG_INVERT LPA_LOG("INVERT:"); LPA_LOG
#else
#define LPA_INT_LOG_INVERT if (0) LPA_LOG
#endif

#if LPA_INT_DEBUG_TOHEX && LPA_INT_DEBUG
#define LPA_INT_LOG_TOHEX LPA_LOG("TOHEX:"); LPA_LOG
#else
#define LPA_INT_LOG_TOHEX if (0) LPA_LOG
#endif

#if LPA_INT_DEBUG_FROMHEX && LPA_INT_DEBUG
#define LPA_INT_LOG_FROMHEX LPA_LOG("FROMHEX:"); LPA_LOG
#else
#define LPA_INT_LOG_FROMHEX if (0) LPA_LOG
#endif

#if LPA_INT_DEBUG_FROMUINT64 && LPA_INT_DEBUG
#define LPA_INT_LOG_FROMUINT64 LPA_LOG("FROMUINT64:"); LPA_LOG
#else
#define LPA_INT_LOG_FROMUINT64 if (0) LPA_LOG
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

static void LPA_INT_copyNumber(LPA_INT_number* pDst, const LPA_INT_number* const pSrc)
{
	const LPA_INT_size numDigits = pSrc->numDigits;
	const LPA_INT_size memorySize = (LPA_INT_size)(numDigits * sizeof(LPA_INT_digit));
	if (pDst->pDigits != NULL)
	{
		LPA_freeMem(pDst->pDigits);
	}
	pDst->pDigits = LPA_allocMem(memorySize);
	memcpy(pDst->pDigits, pSrc->pDigits, memorySize);
	pDst->numDigits = numDigits;
}

static LPA_INT_size LPA_INT_length(const LPA_INT_number* const pNumber)
{
	LPA_INT_size index = 0;
	LPA_INT_size length = 0;
	LPA_INT_size numDigits = pNumber->numDigits;
	LPA_INT_size i = 0;

	for (i = 0; i < numDigits; ++i)
	{
		const LPA_INT_digit value = pNumber->pDigits[i];
		++index;
		if (value != 0)
		{
			if (index > length)
			{
				length = index;
			}
		}
	}
	return length;
}

static void LPA_INT_zeroNumber(LPA_INT_number* const pNumber)
{
	LPA_freeMem(pNumber->pDigits);
	pNumber->pDigits = NULL;
	pNumber->numDigits = 0;
}

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
		if (pOldDigits != NULL)
		{
			memcpy(pNumber->pDigits, pOldDigits, oldMemorySize);
			LPA_freeMem((void*)pOldDigits);
		}
		memset(pNumber->pDigits+oldNumDigits, 0, newMemorySize-oldMemorySize);
	}
}

static void LPA_INT_allocNumber(LPA_INT_number* const pNumber, const LPA_INT_size newNumDigits)
{
	const size_t newMemorySize = newNumDigits * sizeof(LPA_INT_digit);
	if (pNumber == NULL)
	{
		LPA_ERROR("LPA_INT_allocNumber::pNumber is NULL");
		return;
	}
	if (newNumDigits != pNumber->numDigits)
	{
		if (pNumber->pDigits != NULL)
		{
			LPA_freeMem(pNumber->pDigits);
		}
		pNumber->pDigits = LPA_allocMem(newMemorySize);
		pNumber->numDigits = newNumDigits;
	}
	memset(pNumber->pDigits, 0, newMemorySize);
}

static void LPA_INT_addInternal(LPA_INT_number* const pResult, const LPA_INT_number* const pA, const LPA_INT_number* const pB)
{
	LPA_INT_size i = 0;
	const LPA_INT_size aNumDigits = pA->numDigits;
	const LPA_INT_size bNumDigits = pB->numDigits;
	/* sum length : extend if needed for the carry */
	const LPA_INT_size sumSize = ((aNumDigits > bNumDigits) ? aNumDigits : bNumDigits);
	LPA_INT_digit carry = 0;

	LPA_INT_LOG_ADD("sumSize:%d\n", sumSize);
	LPA_INT_allocNumber(pResult, sumSize);

	for (i = 0; i < sumSize; ++i)
	{
		LPA_INT_digit aDigit = 0;
		LPA_INT_digit bDigit = 0;
		LPA_INT_digit sumDigit = 0;
		LPA_INT_LOG_ADD("i:%d\n", i);

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
			LPA_INT_LOG_ADD("carry:%u sumValue:%X aValue:%X bValue:%X\n", carry, sumValue, aValue, bValue);

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
	return;
}

static int LPA_INT_subtractInternal(LPA_INT_number* const pResult, const LPA_INT_number* const pA, const LPA_INT_number* const pB)
{
	LPA_INT_size i = 0;
	const LPA_INT_size aNumDigits = pA->numDigits;
	const LPA_INT_size bNumDigits = pB->numDigits;
	/* sum length : extend if needed for the carry */
	const LPA_INT_size sumSize = ((aNumDigits > bNumDigits) ? aNumDigits : bNumDigits);
	LPA_INT_digit borrow = 0;

	LPA_INT_LOG_SUBTRACT("sumSize:%d\n", sumSize);
	LPA_INT_allocNumber(pResult, sumSize);

	for (i = 0; i < sumSize; ++i)
	{
		LPA_INT_digit aDigit = 0;
		LPA_INT_digit bDigit = 0;
		LPA_INT_digit sumDigit = 0;
		LPA_INT_LOG_SUBTRACT("i:%d\n", i);

		if (i < aNumDigits)
		{
			aDigit = pA->pDigits[i];
		}
		if (i < bNumDigits)
		{
			bDigit = pB->pDigits[i];
		}
		/* digit subtract e.g. base precision arithmetic */
		{
			const LPA_INT_digit aValue = aDigit;
			const LPA_INT_digit bValue = bDigit;
			LPA_INT_digit temp = aValue - borrow;
			LPA_INT_digit sumValue;
			LPA_INT_LOG_SUBTRACT("borrow:%u sumValue:%X aValue:%X bValue:%X\n", borrow, sumValue, aValue, bValue);

			if (temp > aValue)
			{
				borrow = 1;
			}
			else
			{
				borrow = 0;
			}
			sumValue = temp - bValue;
			if (sumValue > temp)
			{
				borrow = 1;
			}
			sumDigit = (LPA_INT_digit)sumValue;
		}
		pResult->pDigits[i] = sumDigit;
	}
	/* Negative if borrow is set */
	return (int)borrow;
}

static void LPA_INT_multiplyInternal(const LPA_INT_number* const pA, const LPA_INT_number* const pB, LPA_INT_number* const pResult)
{
	LPA_INT_size i = 0;
	const LPA_INT_size aNumDigits = pA->numDigits;
	const LPA_INT_size bNumDigits = pB->numDigits;
	/* multiply length : maximum length */
	const LPA_INT_size outNumDigits = (aNumDigits + bNumDigits);
	LPA_INT_digit carry = 0;

	LPA_INT_allocNumber(pResult, outNumDigits);

	for (i = 0; i < aNumDigits; ++i)
	{
		LPA_INT_size outIndex = 0;
		LPA_INT_digitIntermediate aValue = 0;
		LPA_INT_size j = 0;

		carry = 0;
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
			LPA_INT_LOG_MULTIPLY("i:%d j:%d ind:%d a:0x%lX b:0x%lX mult:0x%lX result:0x%X\n", 
					i, j, outIndex, aValue, bValue, multValue, result);
			multValue += result;

			units = (LPA_INT_digit)(multValue & LPA_INT_DIGIT_MASK);
			carry = (LPA_INT_digit)(multValue >> LPA_INT_NUM_BITS_PER_DIGIT);
			pResult->pDigits[outIndex] = units;
			LPA_INT_LOG_MULTIPLY("ind:%d units:0x%X carry:0x%X\n", outIndex, units, carry);
		}
		pResult->pDigits[outIndex+1] = carry;
	}
	return;
}

static void LPA_INT_singleMultiply(LPA_INT_number* const pResult, const LPA_INT_number* const pA, const LPA_INT_digit b)
{
	LPA_INT_size i = 0;
	const LPA_INT_size aNumDigits = pA->numDigits;
	const LPA_INT_size bNumDigits = 1;
	/* multiply length : maximum length */
	const LPA_INT_size outNumDigits = (aNumDigits + bNumDigits);
	const	LPA_INT_digitIntermediate bValue = (LPA_INT_digitIntermediate)b;
	LPA_INT_size outPosition = 0;

	if (aNumDigits == 0)
	{
		LPA_INT_zeroNumber(pResult);
		return;
	}

	LPA_INT_allocNumber(pResult, outNumDigits);

	for (i = 0; i < aNumDigits; ++i)
	{
		LPA_INT_digit carry = 0;
		LPA_INT_digit units;
		LPA_INT_digitIntermediate aValue = pA->pDigits[i];
		LPA_INT_digitIntermediate result = pResult->pDigits[outPosition];
		LPA_INT_digitIntermediate multValue = aValue * bValue + carry;

		multValue += result;

		units = (LPA_INT_digit)(multValue & LPA_INT_DIGIT_MASK);
		carry = (LPA_INT_digit)(multValue >> LPA_INT_NUM_BITS_PER_DIGIT);

		pResult->pDigits[outPosition] = units;
		++outPosition;
		pResult->pDigits[outPosition] = carry;
	}
}

static void LPA_INT_singleDivide(LPA_INT_number* const pQuotient, LPA_INT_number* const pRemainder, 
																	const LPA_INT_number* const pA, const LPA_INT_digit b)
{
	LPA_INT_size jLoop;
	LPA_INT_size aSize;
	LPA_INT_size outIndex;
	LPA_INT_digitIntermediate q = 0;
	LPA_INT_digitIntermediate aPartial = 0;
	LPA_INT_number temp1;
	LPA_INT_number bWork;
#if LPA_INT_DEBUG_DIVIDE
	char outBuffer[CHAR_BUFFER_SIZE];
#endif

	aSize = LPA_INT_length(pA);
	LPA_INT_initNumber(pQuotient);
	LPA_INT_allocNumber(pQuotient, aSize);
	outIndex = aSize - 1;
	for (jLoop = 0; jLoop < aSize; ++jLoop, --outIndex)
	{
		LPA_INT_size j = outIndex;
		LPA_INT_digit aJ = pA->pDigits[j];
		aPartial = (aPartial << LPA_INT_NUM_BITS_PER_DIGIT) + aJ;
		q = aPartial / b;
		pQuotient->pDigits[outIndex] = (LPA_INT_digit)q;
		aPartial -= (b * q);
	}
	LPA_INT_fromInt64(&bWork, b);
	LPA_INT_initNumber(&temp1);
	LPA_INT_multiply(&temp1, pQuotient, &bWork);
#if LPA_INT_DEBUG_DIVIDE
	LPA_INT_toHexadecimalASCII(outBuffer, CHAR_BUFFER_SIZE, pA);
	LPA_INT_LOG_DIVIDE("Single pA:0x%s\n", outBuffer);
	LPA_INT_toHexadecimalASCII(outBuffer, CHAR_BUFFER_SIZE, &bWork);
	LPA_INT_LOG_DIVIDE("Single pB:0x%s\n", outBuffer);
	LPA_INT_toHexadecimalASCII(outBuffer, CHAR_BUFFER_SIZE, pQuotient);
	LPA_INT_LOG_DIVIDE("Single pQuotient:0x%s\n", outBuffer);
	LPA_INT_toHexadecimalASCII(outBuffer, CHAR_BUFFER_SIZE, &temp1);
	LPA_INT_LOG_DIVIDE("Single temp1:0x%s\n", outBuffer);
#endif
	LPA_INT_subtract(pRemainder, pA, &temp1);

	LPA_INT_freeNumber(&temp1);
	LPA_INT_freeNumber(&bWork);
}

/* Bit twiddling hacks function to find log2 fast */
static LPA_uint32 LPA_INT_log2(const LPA_INT_digit value)
{
	static const LPA_uint32 LPA_INT_MultiplyDeBruijnBitPosition[32] = 
	{
		0, 9, 1, 10, 13, 21, 2, 29, 11, 14, 16, 18, 22, 25, 3, 30,
 		8, 12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26, 5, 4, 31
	};

	LPA_uint32 v = value;
	LPA_INT_digit r = 0;

	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;

	r = LPA_INT_MultiplyDeBruijnBitPosition[(LPA_uint32)(v * 0x07C4ACDDu) >> 27];
	return r;
}

static LPA_INT_digit LPA_int_findLog2D(const LPA_INT_digit B1)
{
	/* need to find a D such that B[1]*D >= base/2 */
	/* Choose D  = 2^((NUM_BITS_PER_DIGIT-1) - log2(B[1])) */
	LPA_uint32 log2B1 = LPA_INT_log2(B1);
	LPA_INT_digit log2D = (LPA_INT_NUM_BITS_PER_DIGIT-1 - log2B1);
	LPA_INT_digit D = (LPA_INT_digit)(1u << log2D);

	if (B1*D < (1<<16))
	{
		printf("D:0x%X B1:0x%X\n", D, B1);
		LPA_ERROR("bad D\n");
		return 0;
	}

	return log2D;
}

/* This function isn't optimised yet */
static void LPA_INT_divideInternal(LPA_INT_number* const pQuotient, LPA_INT_number* const pRemainder,
																		const LPA_INT_number* const pA, const LPA_INT_number* const pB)
{
	/* Q = A/B */
	/* R = A mod B */
	LPA_INT_size aSize;
	LPA_INT_size bSize;
	LPA_INT_size jLoop;
	LPA_INT_size i;
	LPA_INT_size m;
	LPA_INT_size mAllocSize;
	LPA_INT_size n;
	LPA_INT_size aWorkLen;
	LPA_INT_digit B1;
	LPA_INT_number aWork;
	LPA_INT_number bWork;
	LPA_INT_number temp1;
	LPA_INT_number temp2;
	LPA_INT_number temp3;
	LPA_INT_digit bWorkNm1;
	LPA_INT_digit bWorkNm2;
	LPA_INT_digit log2D;
	LPA_INT_digit spD;
	int borrow;
#if LPA_INT_DEBUG_DIVIDE
	char outBuffer[CHAR_BUFFER_SIZE];
#endif

	aSize = LPA_INT_length(pA);
	bSize = LPA_INT_length(pB);

	/* n = B_size */
	n = bSize;

	LPA_INT_LOG_DIVIDE("aSize:%d bSize:%d\n", aSize, bSize);
	if (n == 0)
	{
		/* divide by zero */
		LPA_INT_zeroNumber(pQuotient);
		LPA_INT_zeroNumber(pRemainder);
		return;
	}
	if (n == 1)
	{
		/* single digit divide */
		const LPA_INT_digit b = pB->pDigits[0];
		LPA_INT_singleDivide(pQuotient, pRemainder, pA, b);
		return;
	}

	if (bSize > aSize)
	{
		/* quotient = zero */
		LPA_INT_zeroNumber(pQuotient);
		/* remainder = A */
		LPA_INT_copyNumber(pRemainder, pA);
		return;
	}

	LPA_INT_initNumber(&aWork);
	LPA_INT_initNumber(&bWork);
	LPA_INT_initNumber(&temp1);
	LPA_INT_initNumber(&temp2);
	LPA_INT_initNumber(&temp3);
	LPA_INT_initNumber(pQuotient);
	LPA_INT_initNumber(pRemainder);

	/* m = A_size - n */
	m = aSize - n;
	mAllocSize = m + 1;
	LPA_INT_allocNumber(pQuotient, mAllocSize);

	LPA_INT_LOG_DIVIDE("m:%d n:%d\n", m, n);

	/* D1. Normalize: D = <base> / (B[n-1]+1) */
	/* D can be arbitrary as long as B[1]*D >= base/2 e.g. D can made into a power of 2 for power of 2 bases */
	B1 = pB->pDigits[n-1];
	log2D = LPA_int_findLog2D(B1);
	spD = (1u << log2D);
	LPA_INT_fromUint64(&temp1, spD);

	LPA_INT_LOG_DIVIDE("B1:%X spD:%X\n", B1, spD);

	/* Awork = A * D : must always add a leading digit */
	LPA_INT_multiply(&aWork, pA, &temp1);
	aWorkLen = LPA_INT_length(&aWork);

#if LPA_INT_DEBUG_DIVIDE
	LPA_INT_toHexadecimalASCII(outBuffer, CHAR_BUFFER_SIZE, &aWork);
	LPA_INT_LOG_DIVIDE("aWork:%s aSize:%d aWorkLen:%d\n", outBuffer, aSize, aWorkLen);
#endif
	if (LPA_INT_length(pA) == aWorkLen)
	{
		LPA_INT_LOG_DIVIDE("extend\n");
		LPA_INT_extendNumber(&aWork, 1);
		aWorkLen++;
	}
	/* Bwork = B * D */
	LPA_INT_multiply(&bWork, pB, &temp1);
#if LPA_INT_DEBUG_DIVIDE
	LPA_INT_toHexadecimalASCII(outBuffer, CHAR_BUFFER_SIZE, &bWork);
	LPA_INT_LOG_DIVIDE("bWork:%s\n", outBuffer);
#endif

	LPA_INT_freeNumber(&temp1);

	bWorkNm1 = bWork.pDigits[n-1];
	bWorkNm2 = bWork.pDigits[n-2];

	LPA_INT_allocNumber(&temp2, (n+1));

	jLoop = 0;
	do
	{
		LPA_INT_size j = aWorkLen - jLoop - 1;
		LPA_INT_digit qUnit = (1ul << LPA_INT_NUM_BITS_PER_DIGIT) - 1;
		LPA_INT_digitIntermediate aWorkJ = aWork.pDigits[j];
		LPA_INT_digitIntermediate aWorkJm1 = (j > 0) ? aWork.pDigits[j-1] : 0;
		LPA_INT_digitIntermediate aWorkJm2 = (j > 1) ? aWork.pDigits[j-2] : 0;

		LPA_INT_LOG_DIVIDE("jLoop:%u j:%d aWorkJ:%lX bWorkNm1:%X\n", jLoop, j, aWorkJ, bWorkNm1);
		/* D3. Calculate Qunit: if Awork[j] == Bwork[1], Qunit = base - 1 else Qunit = (Awork[j] * base + Awork[j+1]) / Bwork[1] */
		if (aWorkJ != bWorkNm1)
		{
			qUnit = (LPA_INT_digit)(((aWorkJ << LPA_INT_NUM_BITS_PER_DIGIT) + aWorkJm1) / bWorkNm1);
			LPA_INT_LOG_DIVIDE("qUnit: %X jLoop:%u aWorkJm1:%lX\n", qUnit, jLoop, aWorkJm1);
		}
		/* if (Bwork[2] * qUnit > (Awork[j] * 2^LPA_INT_NUM_BITS_PER_DIGIT + Awork[j+1] - qUnit * Bwork[1])* 2^LPA_INT_NUM_BITS_PER_DIGIT + Awork[j+2] qUnit -=1 and test again */
		if ((bWorkNm2 * qUnit) > ((((aWorkJ << LPA_INT_NUM_BITS_PER_DIGIT) + aWorkJm1) - qUnit * bWorkNm1) << LPA_INT_NUM_BITS_PER_DIGIT) + aWorkJm2)
		{
			--qUnit;
			LPA_INT_LOG_DIVIDE("qUnit: %X one too big\n", qUnit);
			if ((bWorkNm2 * qUnit) > ((((aWorkJ << LPA_INT_NUM_BITS_PER_DIGIT) + aWorkJm1) - qUnit * bWorkNm1) << LPA_INT_NUM_BITS_PER_DIGIT) + aWorkJm2)
			{
				--qUnit;
				LPA_INT_LOG_DIVIDE("qUnit: %X two too big\n", qUnit);
			}
		}

		/* if (Bwork[2] * qUnit > Awork[j] * 2^LPA_INT_NUM_BITS_PER_DIGIT + Awork[j+1] - qUnit * Bwork[1] qUnit -=1 */
		/* D4. Multiply & Subtract: Awork[j...j+n] = A[j...j+n] - Qunit * Bwork, save the borrow flag if Awork is -ve */
		LPA_INT_singleMultiply(&temp1, &bWork, (LPA_INT_digit)qUnit);
#if LPA_INT_DEBUG_DIVIDE
		LPA_INT_LOG_DIVIDE("qUnit: 0x%X n:%d j:%d jLoop:%d n-j:%d\n", qUnit, n, j, jLoop, n-j);
		LPA_INT_toHexadecimalASCII(outBuffer, CHAR_BUFFER_SIZE, &temp1);
		LPA_INT_LOG_DIVIDE("temp1: %s\n", outBuffer);
#endif

		LPA_INT_LOG_DIVIDE("numTempDigits:%d n:%d jLoop:%d\n", n+1, n, jLoop);
		if (aSize-jLoop <= aSize)
		{
			for (i = 0; i < (n+1); ++i)
			{
				LPA_INT_size dstDigit = n-i;
				LPA_INT_size srcDigit = (aSize-1) - i - jLoop + 1;
				LPA_INT_digit src = aWork.pDigits[srcDigit];
				LPA_INT_LOG_DIVIDE("src[%d] temp2[%d]: 0x%X\n", srcDigit, dstDigit, src);
				temp2.pDigits[dstDigit] = src;
			}
		}
		else
		{
			/* This an error */
			LPA_ERROR("INT divide: not enough digits for remainder\n");
			LPA_INT_zeroNumber(pQuotient);
			LPA_INT_zeroNumber(pRemainder);
			return;
		}
#if LPA_INT_DEBUG_DIVIDE
		LPA_INT_toHexadecimalASCII(outBuffer, CHAR_BUFFER_SIZE, &temp2);
		LPA_INT_LOG_DIVIDE("temp2: %s\n", outBuffer);
#endif

		borrow = LPA_INT_subtractInternal(&temp3, &temp2, &temp1);
#if LPA_INT_DEBUG_DIVIDE
		LPA_INT_toHexadecimalASCII(outBuffer, CHAR_BUFFER_SIZE, &temp3);
		LPA_INT_LOG_DIVIDE("temp3: %s\n", outBuffer);
#endif
		/* D5. if borrow flag then Q[j] -= 1 & invert Awork,  then Awork = Qunit * Bwork - A[j...j+n] */
		if (borrow)
		{
#if LPA_INT_DEBUG_DIVIDE
			LPA_INT_toHexadecimalASCII(outBuffer, CHAR_BUFFER_SIZE, &temp3);
			LPA_INT_LOG_DIVIDE("negative temp3: %s\n", outBuffer);
#endif
			--qUnit;
			borrow = LPA_INT_subtractInternal(&temp3, &temp1, &temp2);
			if (borrow)
			{
				LPA_ERROR("temp3 still negative\n");
			}
#if LPA_INT_DEBUG_DIVIDE
			LPA_INT_toHexadecimalASCII(outBuffer, CHAR_BUFFER_SIZE, &temp3);
			LPA_INT_LOG_DIVIDE("negative temp3: %s\n", outBuffer);
#endif
		}

		for (i = 0; i < (n+1); ++i)
		{
			LPA_INT_size srcDigit = n-i;
			LPA_INT_size dstDigit = (aSize-1) - i - jLoop + 1;
			LPA_INT_digit src = temp3.pDigits[srcDigit];
			LPA_INT_LOG_DIVIDE("src[%d] aWork[%d]: 0x%X\n", srcDigit, dstDigit, src);
			aWork.pDigits[dstDigit] = src;
		}
#if LPA_INT_DEBUG_DIVIDE
		LPA_INT_toHexadecimalASCII(outBuffer, CHAR_BUFFER_SIZE, &aWork);
		LPA_INT_LOG_DIVIDE("aWork: %s\n", outBuffer);
#endif

		/* D6. Set Q[j] = Qunit */
		LPA_INT_LOG_DIVIDE("pQuotient[%d]: 0x%X\n", mAllocSize - 1 - jLoop, qUnit);
		pQuotient->pDigits[mAllocSize-1-jLoop] = (LPA_INT_digit)qUnit;
		++jLoop;
	}
	while (jLoop <= m);
	/* D8. Q = u/v, R = Awork[m+1..m+n]/D */
	LPA_INT_singleDivide(pRemainder, &temp2, &aWork, (LPA_INT_digit)spD);

	LPA_INT_freeNumber(&aWork);
	LPA_INT_freeNumber(&bWork);
	LPA_INT_freeNumber(&temp1);
	LPA_INT_freeNumber(&temp2);
	LPA_INT_freeNumber(&temp3);
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
	LPA_INT_zeroNumber(pNumber);
}

void LPA_INT_toDecimalASCII(char* const pBuffer, const size_t maxNumChars, const LPA_INT_number* const pNumber)
{
	LPA_INT_size i;
	size_t outIndex = 0;
	const LPA_INT_size maxLoop = pNumber->numDigits;
	size_t maxValidChar = 0;
	int validNumber = 0;
	LPA_INT_number number;
	LPA_INT_number remainder;
	LPA_INT_size len = 0;

	if (maxLoop == 0)
	{
		pBuffer[0] = '0';
		pBuffer[1] = '\0';
		return;
	}

	LPA_INT_initNumber(&number);
	LPA_INT_copyNumber(&number, pNumber);

	do
	{
		LPA_INT_number work;
		char c = '*';
		LPA_INT_digitIntermediate value = 0;
		LPA_INT_initNumber(&work);
		LPA_INT_initNumber(&remainder);
		LPA_INT_singleDivide(&work, &remainder, &number, 10);
		if (remainder.numDigits > 0)
		{
			value = remainder.pDigits[0];
		}

		LPA_INT_copyNumber(&number, &work);
		LPA_INT_freeNumber(&work);
		LPA_INT_freeNumber(&remainder);

		c = (char)('0' + value);
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

		len = LPA_INT_length(&number);
	} while (len > 0);

	LPA_INT_freeNumber(&number);
	LPA_INT_freeNumber(&remainder);

	if (validNumber == 0)
	{
		pBuffer[0] = '0';
		maxValidChar = 0;
	}
	outIndex = maxValidChar+1;
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

void LPA_INT_toHexadecimalASCII(char* const pBuffer, const size_t maxNumChars, const LPA_INT_number* const pNumber)
{
	LPA_INT_size i;
	size_t outIndex = 0;
	const LPA_INT_size maxLoop = pNumber->numDigits;
	size_t maxValidChar = 0;

	if (maxLoop == 0)
	{
		pBuffer[0] = '0';
		pBuffer[1] = '\0';
		return;
	}
	LPA_INT_LOG_TOHEX("maxLoop:%u\n", maxLoop);
	for (i = 0; i < maxLoop; ++i)
	{
		int j;
		LPA_INT_digit digit = pNumber->pDigits[i];
		LPA_INT_LOG_TOHEX("digit[%d]:0x%X\n", i, digit);

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

void LPA_INT_fromDecimalASCII(LPA_INT_number* const pNumber, const char* const value)
{
	const char* pStr = value;
	LPA_INT_number base;

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
		if ((c == '-') || (c == '+') || ((c >= '0') && (c <= '9')))
		{
			break;
		}
		++pStr;
	}

	LPA_INT_fromUint64(&base, 10);
	do
	{
		LPA_INT_number temp1;
		LPA_INT_number temp2;
		const char c = *pStr;
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
		if (digit > 10)
		{
			LPA_INT_freeNumber(pNumber);
			LPA_INT_freeNumber(&base);
			return;
		}
		LPA_INT_fromUint32(&temp2, digit);
		LPA_INT_initNumber(&temp1);
		LPA_INT_multiply(&temp1, pNumber, &base);
		LPA_INT_add(pNumber, &temp1, &temp2);
		LPA_INT_freeNumber(&temp1);
		LPA_INT_freeNumber(&temp2);
		++pStr;
	}
	while (*pStr != '\0');
	LPA_INT_freeNumber(&base);
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
			LPA_INT_freeNumber(pNumber);
			return;
		}
		LPA_INT_LOG_FROMHEX("c:%c index:%d digit:%u\n", c, index, digit);
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

void LPA_INT_add(LPA_INT_number* const pResult, const LPA_INT_number* const pA, const LPA_INT_number* const pB)
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
	LPA_INT_addInternal(pResult, pA, pB);
	return;
}

void LPA_INT_subtract(LPA_INT_number* const pResult, const LPA_INT_number* const pA, const LPA_INT_number* const pB)
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
	LPA_INT_subtractInternal(pResult, pA, pB);
	return;
}

void LPA_INT_multiply(LPA_INT_number* const pResult, const LPA_INT_number* const pA, const LPA_INT_number* const pB)
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
	return;
}

void LPA_INT_divide(LPA_INT_number* const pQuotient, LPA_INT_number* const pRemainder, 
										const LPA_INT_number* const pA, const LPA_INT_number* const pB)
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
	LPA_INT_divideInternal(pQuotient, pRemainder, pA, pB);
}

