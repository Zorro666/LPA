#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#include "lpa.h"

void* LPA_allocMem(size_t numBytes)
{
	return malloc(numBytes);
}

void LPA_freeMem(void* pMem)
{
	free(pMem);
}

typedef enum {TEST_RANDOM, TEST_SEQUENTIAL} TestMode;

#define CHAR_BUFFER_SIZE (2048)

typedef void(LPA_BCD_func1)(LPA_BCD_number* const pResult, const LPA_BCD_number* const pA, const LPA_BCD_number* const pB);
typedef void(LPA_BCD_func2)(LPA_BCD_number* const pResult1, LPA_BCD_number* const pResult2, 
														const LPA_BCD_number* const pA, const LPA_BCD_number* const pB);

typedef void(LPA_INT_func1)(LPA_INT_number* const pResult, const LPA_INT_number* const pA, const LPA_INT_number* const pB);
typedef void(LPA_INT_func2)(LPA_INT_number* const pResult1, LPA_INT_number* const pResult2,
														const LPA_INT_number* const pA, const LPA_INT_number* const pB);

static void LPA_INT_multiplyDivide(LPA_INT_number* const pQuotient, LPA_INT_number* const pRemainder,
																		const LPA_INT_number* const pA, const LPA_INT_number* const pB)
{
	LPA_INT_number temp1;
	LPA_INT_number temp2;
	LPA_INT_number temp3;
	LPA_INT_initNumber(&temp1);
	LPA_INT_initNumber(&temp2);
	LPA_INT_initNumber(&temp3);

	/* t1 = (a * b) */
	LPA_INT_multiply(&temp1, pA, pB);
	/* t2 = ((a * b) * b) */
	LPA_INT_multiply(&temp2, &temp1, pB); 
	/* t3 = ((a * b) * ((a * b) * b))) */
	LPA_INT_multiply(&temp3, &temp1, &temp2);
	/* t1 = ((a * b) * b) * (((a * b) * ((a * b) * b)))) */
	LPA_INT_freeNumber(&temp1);
	LPA_INT_multiply(&temp1, &temp2, &temp3);
	/* q = ((a * b) * b) * (((a * b) * ((a * b) * b)))) / (((a * b) * ((a * b) * b)))) */
	/* q = ((a * b) * b) */
	LPA_INT_divide(pQuotient, pRemainder, &temp1, &temp3);
	/* q = ((a * b) * b) / b */
	/* t1 = (a * b) */
	LPA_INT_freeNumber(&temp1);
	LPA_INT_divide(&temp1, &temp2, pQuotient, pB);
	/* q = (a * b) / b */
	LPA_INT_freeNumber(pQuotient);
	LPA_INT_freeNumber(pRemainder);
	LPA_INT_divide(pQuotient, pRemainder, &temp1, pB);

	LPA_INT_freeNumber(&temp1);
	LPA_INT_freeNumber(&temp2);
	LPA_INT_freeNumber(&temp3);
}

static int doLPA_BCDTest1(const char* const test, LPA_BCD_func1 testFunc, const long a, const long b, const int verbose)
{
	LPA_BCD_number lpaResult;
	LPA_BCD_number longResult;
	LPA_BCD_number lpaA;
	LPA_BCD_number lpaB;
	char resultTruth[CHAR_BUFFER_SIZE];
	char outResult[CHAR_BUFFER_SIZE];
	char outA[CHAR_BUFFER_SIZE];
	char outB[CHAR_BUFFER_SIZE];
	long result = 0;
	int success = 1;

	LPA_BCD_initNumber(&longResult);
	LPA_BCD_initNumber(&lpaResult);
	LPA_BCD_initNumber(&lpaA);
	LPA_BCD_initNumber(&lpaB);

	LPA_BCD_fromInt64(&lpaA, a);
	LPA_BCD_fromInt64(&lpaB, b);

	if (testFunc == LPA_BCD_add)
	{
		result = a + b;
	}
	else if (testFunc == LPA_BCD_subtract)
	{
		result = a - b;
	}
	else if (testFunc == LPA_BCD_multiply)
	{
		result = a * b;
	}
	LPA_BCD_fromInt64(&longResult, result);

	testFunc(&lpaResult, &lpaA, &lpaB);

	LPA_BCD_toDecimalASCII(outA, CHAR_BUFFER_SIZE, &lpaA);
	LPA_BCD_toDecimalASCII(outB, CHAR_BUFFER_SIZE, &lpaB);
	LPA_BCD_toDecimalASCII(outResult, CHAR_BUFFER_SIZE, &lpaResult);
	if (verbose)
	{
		printf("%s:\tBCD: %s long: %ld\n", test, outResult, result);
		printf("%s:\tBCD: %s %s %s = %s long: %ld %s %ld = %ld\n", 
				test, outA, test, outB, outResult, a, test, b, result);
	}

	LPA_BCD_toHexadecimalASCII(outResult, CHAR_BUFFER_SIZE, &lpaResult);
	sprintf(resultTruth, "%s%lX", (result < 0) ? "-" : "", (result < 0) ? -result : result);
	if (strcmp(resultTruth, outResult) != 0)
	{
		success = 0;
		fprintf(stderr, "TEST FAILED '%s' : %ld %s %ld = %s != %s\n", test, a, test, b, resultTruth, outResult);
	}

	LPA_BCD_toDecimalASCII(outResult, CHAR_BUFFER_SIZE, &lpaResult);
	sprintf(resultTruth, "%ld", result);
	if (strcmp(resultTruth, outResult) != 0)
	{
		success = 0;
		fprintf(stderr, "TEST FAILED '%s' : %ld %s %ld = %s != %s\n", test, a, test, b, resultTruth, outResult);
	}

	LPA_BCD_toHexadecimalASCII(outResult, CHAR_BUFFER_SIZE, &lpaResult);
	LPA_BCD_toHexadecimalASCII(resultTruth, CHAR_BUFFER_SIZE, &longResult);
	sprintf(resultTruth, "%s%lX", (result < 0) ? "-" : "", (result < 0) ? -result : result);
	if (strcmp(resultTruth, outResult) != 0)
	{
		success = 0;
		fprintf(stderr, "TEST FAILED '%s' : %ld %s %ld = %s != %s\n", test, a, test, b, resultTruth, outResult);
	}

	LPA_BCD_toDecimalASCII(outResult, CHAR_BUFFER_SIZE, &lpaResult);
	LPA_BCD_toDecimalASCII(resultTruth, CHAR_BUFFER_SIZE, &longResult);
	if (strcmp(resultTruth, outResult) != 0)
	{
		success = 0;
		fprintf(stderr, "TEST FAILED '%s' : %ld %s %ld = %s != %s\n", test, a, test, b, resultTruth, outResult);
	}

	if (success == 0)
	{
		fprintf(stderr, "%s:\tBCD: %s long: %ld\nBCD: %s %s %s = %s\nlong: %ld %s %ld = %ld\n", 
				test, outResult, result, outA, test, outB, outResult, a, test, b, result);
	}

	LPA_BCD_freeNumber(&longResult);
	LPA_BCD_freeNumber(&lpaResult);
	LPA_BCD_freeNumber(&lpaA);
	LPA_BCD_freeNumber(&lpaB);
	return success;
}

static int doLPA_BCDTest2(const char* const test1, const char* const test2, LPA_BCD_func2 testFunc, 
													const long a, const long b, const int verbose)
{
	LPA_BCD_number lpaResult1;
	LPA_BCD_number lpaResult2;
	LPA_BCD_number lpaA;
	LPA_BCD_number lpaB;
	char result1Truth[CHAR_BUFFER_SIZE];
	char result2Truth[CHAR_BUFFER_SIZE];
	char outResult1[CHAR_BUFFER_SIZE];
	char outResult2[CHAR_BUFFER_SIZE];
	char outA[CHAR_BUFFER_SIZE];
	char outB[CHAR_BUFFER_SIZE];
	long result1 = 0;
	long result2 = 0;
	int success1 = 1;
	int success2 = 1;

	LPA_BCD_initNumber(&lpaResult1);
	LPA_BCD_initNumber(&lpaResult2);
	LPA_BCD_initNumber(&lpaA);
	LPA_BCD_initNumber(&lpaB);

	if (testFunc == LPA_BCD_divide)
	{
		if (b == 0)
		{
			return 1;
		}
		result1 = a / b;
		result2 = a % b;
	}

	LPA_BCD_fromInt64(&lpaA, a);
	LPA_BCD_fromInt64(&lpaB, b);

	testFunc(&lpaResult1, &lpaResult2, &lpaA, &lpaB);

	LPA_BCD_toDecimalASCII(outA, CHAR_BUFFER_SIZE, &lpaA);
	LPA_BCD_toDecimalASCII(outB, CHAR_BUFFER_SIZE, &lpaB);
	LPA_BCD_toDecimalASCII(outResult1, CHAR_BUFFER_SIZE, &lpaResult1);
	LPA_BCD_toDecimalASCII(outResult2, CHAR_BUFFER_SIZE, &lpaResult2);
	if (verbose)
	{
		printf("%s:\tBCD: %s long: %ld\n", test1, outResult1, result1);
		printf("%s:\tBCD: %s %s %s = %s long: %ld %s %ld = %ld\n", 
				test1, outA, test1, outB, outResult1, a, test1, b, result1);
	}

	sprintf(result1Truth, "%ld", result1);
	if (strcmp(result1Truth, outResult1) != 0)
	{
		success1 = 0;
	}

	LPA_BCD_freeNumber(&lpaResult1);
	LPA_BCD_fromInt64(&lpaResult1, result1);
	LPA_BCD_toDecimalASCII(result1Truth, CHAR_BUFFER_SIZE, &lpaResult1);
	if (strcmp(result1Truth, outResult1) != 0)
	{
		success1 = 0;
	}
	if (success1 == 0)
	{
		fprintf(stderr, "%s:\tBCD: %s long: %ld\nBCD: %s %s %s = %s\nlong: %ld %s %ld = %ld\n", 
				test1, outResult1, result1, outA, test1, outB, outResult1, a, test1, b, result1);
		fprintf(stderr, "TEST FAILED '%s' : %ld %s %ld = %s != %s\n", test1, a, test1, b, result1Truth, outResult1);
	}

	if (verbose)
	{
		printf("%s:\tBCD: %s long: %ld\nBCD: %s %s %s = %s long: %ld %s %ld = %ld\n", 
				test2, outResult2, result2, outA, test2, outB, outResult2, a, test2, b, result2);
	}
	sprintf(result2Truth, "%ld", result2);
	if (strcmp(result2Truth, outResult2) != 0)
	{
		success2 = 0;
	}

	LPA_BCD_freeNumber(&lpaResult2);
	LPA_BCD_fromInt64(&lpaResult2, result2);
	LPA_BCD_toDecimalASCII(result2Truth, CHAR_BUFFER_SIZE, &lpaResult2);
	if (strcmp(result2Truth, outResult2) != 0)
	{
		success2 = 0;
	}

	if (success2 == 0)
	{
		fprintf(stderr, "%s:\tBCD: %s long: %ld\nBCD: %s %s %s = %s\nlong: %ld %s %ld = %ld\n", 
				test2, outResult2, result2, outA, test2, outB, outResult2, a, test2, b, result2);
		fprintf(stderr, "TEST FAILED '%s' : %ld %s %ld = %s != %s\n", test2, a, test2, b, result2Truth, outResult2);
	}

	LPA_BCD_freeNumber(&lpaResult1);
	LPA_BCD_freeNumber(&lpaResult2);
	LPA_BCD_freeNumber(&lpaA);
	LPA_BCD_freeNumber(&lpaB);

	return success1 && success2;
}

static int doLPA_BCDTests(const long numTests, const TestMode mode)
{
	int success = 1;
	long i = 0;
	long a = 0;
	long b = 0;
	long maxValue = 0;

	if (mode == TEST_SEQUENTIAL)
	{
		maxValue = (long)sqrt((float)numTests);
		printf("maxValue:%ld\n", maxValue);
	}

	while ((i < numTests) && (success == 1))
	{
		if (mode == TEST_RANDOM)
		{
			a = rand() - (RAND_MAX >> 1);
			b = rand() - (RAND_MAX >> 1);
		}
		else if (mode == TEST_SEQUENTIAL)
		{
			b++;
			if (b > maxValue)
			{
				a++;
				b = 0;
			}
		}
		
		success = doLPA_BCDTest1("+", LPA_BCD_add, a, b, 0);
		if (success == 0)
		{
			break;
		}

		success = doLPA_BCDTest1("*", LPA_BCD_multiply, a, b, 0);
		if (success == 0)
		{
			break;
		}

		success = doLPA_BCDTest1("-", LPA_BCD_subtract, a, b, 0);
		if (success == 0)
		{
			break;
		}
		success = doLPA_BCDTest2("/", "%", LPA_BCD_divide, a, b, 0);
		if (success == 0)
		{
			break;
		}

		++i;
	}

	return success;
}

static int doLPA_BCDRandomTests(const long numTests)
{
	printf("################# BCD Random Tests : %lu #################\n", numTests);
	return doLPA_BCDTests(numTests, TEST_RANDOM);
}

static int doLPA_BCDSequentialTests(const long numTests)
{
	printf("################# BCD Sequential Tests : %lu #################\n", numTests);
	return doLPA_BCDTests(numTests, TEST_SEQUENTIAL);
}

static int doLPA_INTTest1(const char* const test, LPA_INT_func1 testFunc, const long a, const long b, const int verbose)
{
	LPA_INT_number longResult;
	LPA_INT_number lpaResult;
	LPA_INT_number lpaA;
	LPA_INT_number lpaB;
	char resultTruth[CHAR_BUFFER_SIZE];
	char outResult[CHAR_BUFFER_SIZE];
	char outA[CHAR_BUFFER_SIZE];
	char outB[CHAR_BUFFER_SIZE];
	long result = 0;
	int success = 1;

	LPA_INT_initNumber(&longResult);
	LPA_INT_initNumber(&lpaResult);
	LPA_INT_initNumber(&lpaA);
	LPA_INT_initNumber(&lpaB);

	LPA_INT_fromInt64(&lpaA, a);
	LPA_INT_fromInt64(&lpaB, b);

	if (testFunc == LPA_INT_add)
	{
		result = a + b;
	}
	else if (testFunc == LPA_INT_subtract)
	{
		result = a - b;
	}
	else if (testFunc == LPA_INT_multiply)
	{
		result = a * b;
	}
	LPA_INT_fromInt64(&longResult, result);

	testFunc(&lpaResult, &lpaA, &lpaB);

	LPA_INT_toHexadecimalASCII(outA, CHAR_BUFFER_SIZE, &lpaA);
	LPA_INT_toHexadecimalASCII(outB, CHAR_BUFFER_SIZE, &lpaB);
	LPA_INT_toHexadecimalASCII(outResult, CHAR_BUFFER_SIZE, &lpaResult);
	if (verbose)
	{
		printf("%s:\tLPA: 0x%s long: 0x%lX\n", test, outResult, result);
		printf("%s:\tLPA: 0x%s %s 0x%s = 0x%s long: 0x%lX %s 0x%lX = 0x%lX\n", 
				test, outA, test, outB, outResult, a, test, b, result);
	}

	LPA_INT_toDecimalASCII(outResult, CHAR_BUFFER_SIZE, &lpaResult);
	sprintf(resultTruth, "%ld", result);
	if (strcmp(resultTruth, outResult) != 0)
	{
		success = 0;
		fprintf(stderr, "DEC TEST FAILED '%s' : 0x%lX %s 0x%lX = %s != %s\n", test, a, test, b, resultTruth, outResult);
	}

	LPA_INT_toHexadecimalASCII(outResult, CHAR_BUFFER_SIZE, &lpaResult);
	sprintf(resultTruth, "%lX", result);
	if (strcmp(resultTruth, outResult) != 0)
	{
		success = 0;
		fprintf(stderr, "TEST FAILED '%s' : 0x%lX %s 0x%lX = 0x%s != 0x%s\n", test, a, test, b, resultTruth, outResult);
	}


	LPA_INT_toDecimalASCII(outResult, CHAR_BUFFER_SIZE, &lpaResult);
	sprintf(resultTruth, "%ld", result);
	if (strcmp(resultTruth, outResult) != 0)
	{
		success = 0;
		fprintf(stderr, "TEST FAILED '%s' : 0x%lX %s 0x%lX = %s != %s\n", test, a, test, b, resultTruth, outResult);
	}

	LPA_INT_toHexadecimalASCII(outResult, CHAR_BUFFER_SIZE, &lpaResult);
	sprintf(resultTruth, "%lX", result);
	if (strcmp(resultTruth, outResult) != 0)
	{
		success = 0;
	}

	LPA_INT_toDecimalASCII(outResult, CHAR_BUFFER_SIZE, &lpaResult);
	LPA_INT_toDecimalASCII(resultTruth, CHAR_BUFFER_SIZE, &longResult);
	if (strcmp(resultTruth, outResult) != 0)
	{
		success = 0;
		fprintf(stderr, "TEST FAILED '%s' : %lX %s %lX = %s != %s\n", test, a, test, b, resultTruth, outResult);
	}

	LPA_INT_toHexadecimalASCII(outResult, CHAR_BUFFER_SIZE, &lpaResult);
	LPA_INT_toHexadecimalASCII(resultTruth, CHAR_BUFFER_SIZE, &longResult);
	if (strcmp(resultTruth, outResult) != 0)
	{
		success = 0;
		fprintf(stderr, "TEST FAILED '%s' : %lX %s %lX = 0x%s != 0x%s\n", test, a, test, b, resultTruth, outResult);
	}

	if (success == 0)
	{
		fprintf(stderr, "%s:\tLPA: 0x%s long: 0x%lX\nLPA: 0x%s %s 0x%s = 0x%s\nlong: 0x%lX %s 0x%lX = 0x%lX\n", 
				test, outResult, result, outA, test, outB, outResult, a, test, b, result);
		fprintf(stderr, "TEST FAILED '%s' : 0x%lX %s 0x%lX = 0x%s != 0x%s\n", test, a, test, b, resultTruth, outResult);
	}

	LPA_INT_freeNumber(&longResult);
	LPA_INT_freeNumber(&lpaResult);
	LPA_INT_freeNumber(&lpaA);
	LPA_INT_freeNumber(&lpaB);
	return success;
}

static int doLPA_INTTest2(const char* const test1, const char* const test2, LPA_INT_func2 testFunc, 
													const long a, const long b, const int verbose)
{
	LPA_INT_number lpaResult1;
	LPA_INT_number lpaResult2;
	LPA_INT_number lpaA;
	LPA_INT_number lpaB;
	char resultTruth[CHAR_BUFFER_SIZE];
	char outResult[CHAR_BUFFER_SIZE];
	char outA[CHAR_BUFFER_SIZE];
	char outB[CHAR_BUFFER_SIZE];
	long result1 = 0;
	long result2 = 0;
	int success1 = 1;
	int success2 = 1;

	LPA_INT_initNumber(&lpaResult1);
	LPA_INT_initNumber(&lpaResult2);
	LPA_INT_initNumber(&lpaA);
	LPA_INT_initNumber(&lpaB);

	if (testFunc == LPA_INT_divide)
	{
		if (b == 0)
		{
			return 1;
		}
		result1 = a / b;
		result2 = a % b;
	}
	if (testFunc == LPA_INT_multiplyDivide)
	{
		if (b == 0)
		{
			return 1;
		}
		result1 = a;
		result2 = 0;
	}

	LPA_INT_fromInt64(&lpaA, a);
	LPA_INT_fromInt64(&lpaB, b);
	testFunc(&lpaResult1, &lpaResult2, &lpaA, &lpaB);

	LPA_INT_toHexadecimalASCII(outA, CHAR_BUFFER_SIZE, &lpaA);
	LPA_INT_toHexadecimalASCII(outB, CHAR_BUFFER_SIZE, &lpaB);

	LPA_INT_toHexadecimalASCII(outResult, CHAR_BUFFER_SIZE, &lpaResult1);
	if (verbose)
	{
		printf("%s:\tLPA: 0x%s long: 0x%lX\n", test1, outResult, result1);
		printf("%s:\tLPA: 0x%s %s 0x%s = 0x%s long: 0x%lX %s 0x%lX = 0x%lX\n", 
				test1, outA, test1, outB, outResult, a, test1, b, result1);
	}

	sprintf(resultTruth, "%lX", result1);
	if (strcmp(resultTruth, outResult) != 0)
	{
		success1 = 0;
	}
	LPA_INT_freeNumber(&lpaResult1);
	LPA_INT_fromInt64(&lpaResult1, result1);
	LPA_INT_toHexadecimalASCII(resultTruth, CHAR_BUFFER_SIZE, &lpaResult1);
	if (strcmp(resultTruth, outResult) != 0)
	{
		success1 = 0;
	}
	if (success1 == 0)
	{
		fprintf(stderr, "%s:\tLPA: 0x%s long: 0x%lX\nLPA: 0x%s %s 0x%s = 0x%s\nlong: 0x%lX %s 0x%lX = 0x%lX\n", 
				test1, outResult, result1, outA, test1, outB, outResult, a, test1, b, result1);
		fprintf(stderr, "TEST FAILED '%s' : 0x%lX %s 0x%lX = 0x%s != 0x%s\n", test1, a, test1, b, resultTruth, outResult);
	}

	LPA_INT_toHexadecimalASCII(outResult, CHAR_BUFFER_SIZE, &lpaResult2);
	if (verbose)
	{
		printf("%s:\tLPA: 0x%s long: 0x%lX\n", test2, outResult, result2);
		printf("%s:\tLPA: 0x%s %s 0x%s = 0x%s long: 0x%lX %s 0x%lX = 0x%lX\n", 
				test2, outA, test2, outB, outResult, a, test2, b, result2);
	}

	sprintf(resultTruth, "%lX", result2);
	if (strcmp(resultTruth, outResult) != 0)
	{
		success2 = 0;
	}
	LPA_INT_freeNumber(&lpaResult2);
	LPA_INT_fromInt64(&lpaResult2, result2);
	LPA_INT_toHexadecimalASCII(resultTruth, CHAR_BUFFER_SIZE, &lpaResult2);
	if (strcmp(resultTruth, outResult) != 0)
	{
		success2 = 0;
	}
	if (success2 == 0)
	{
		fprintf(stderr, "%s:\tLPA: 0x%s long: 0x%lX\nLPA: 0x%s %s 0x%s = 0x%s\nlong: 0x%lX %s 0x%lX = 0x%lX\n", 
				test2, outResult, result2, outA, test2, outB, outResult, a, test2, b, result2);
		fprintf(stderr, "TEST FAILED '%s' : 0x%lX %s 0x%lX = 0x%s != 0x%s\n", test2, a, test2, b, resultTruth, outResult);
	}

	LPA_INT_freeNumber(&lpaResult1);
	LPA_INT_freeNumber(&lpaResult2);
	LPA_INT_freeNumber(&lpaA);
	LPA_INT_freeNumber(&lpaB);

	return success1 && success2;
}

static int doLPA_INTTests(const long numTests, const TestMode mode)
{
	int success = 1;
	long i = 0;
	long a = 0;
	long b = 0;
	long maxValue = 0;

	if (mode == TEST_SEQUENTIAL)
	{
		maxValue = (long)sqrt((float)numTests);
		printf("maxValue:%ld\n", maxValue);
	}

	while ((i < numTests) && (success == 1))
	{
		if (mode == TEST_RANDOM)
		{
			a = rand();
			b = rand();
		}
		else if (mode == TEST_SEQUENTIAL)
		{
			b++;
			if (b > maxValue)
			{
				a++;
				b = 0;
			}
		}
		
		success = doLPA_INTTest1("+", LPA_INT_add, a, b, 0);
		if (success == 0)
		{
			break;
		}

		a = a % INT_MAX;
		b = b % INT_MAX;
		success = doLPA_INTTest1("*", LPA_INT_multiply, a, b, 0);
		if (success == 0)
		{
			break;
		}

		if (b > a)
		{
			long temp = a;
			a = b;
			b = temp;
		}
		success = doLPA_INTTest1("-", LPA_INT_subtract, a, b, 0);
		if (success == 0)
		{
			break;
		}
		success = doLPA_INTTest2("/", "%", LPA_INT_divide, a, b, 0);
		if (success == 0)
		{
			break;
		}
		success = doLPA_INTTest2("*//", "*/%", LPA_INT_multiplyDivide, a, b, 0);
		if (success == 0)
		{
			break;
		}

		++i;
	}

	return success;
}

static int doLPA_INTRandomTests(const long numTests)
{
	printf("################# INT Random Tests : %lu #################\n", numTests);
	return doLPA_INTTests(numTests, TEST_RANDOM);
}

static int doLPA_INTSequentialTests(const long numTests)
{
	printf("################# INT Sequential Tests : %lu #################\n", numTests);
	return doLPA_INTTests(numTests, TEST_SEQUENTIAL);
}

static int testBCD(const int argc, char** argv)
{
	LPA_BCD_number testNumber;
	LPA_BCD_number aNumber;
	LPA_BCD_number resultNumber;
	LPA_BCD_number tempNumber;
	int inA32 = 166;
	long inA = 166;
	long inB = 166;
	long numTests = 1;
	char outBuffer[CHAR_BUFFER_SIZE];
	int i;

	LPA_BCD_initNumber(&testNumber);
	LPA_BCD_initNumber(&aNumber);
	LPA_BCD_initNumber(&resultNumber);
	LPA_BCD_initNumber(&tempNumber);
	if (argc > 1)
	{
		numTests = atol(argv[1]);
	}
	if (argc > 2)
	{
		inA = atol(argv[2]);
	}
	if (argc > 3)
	{
		inB = atol(argv[3]);
	}
	printf("################# BCD Tests : %lu #################\n", numTests);

	printf("inNumber:%ld\n", inA);
	LPA_BCD_fromInt64(&aNumber, inA);
	printf("numDigits:%u\n", aNumber.numDigits);

	LPA_BCD_toDecimalASCII(outBuffer, CHAR_BUFFER_SIZE, &aNumber);
	printf("in:%ld\n", inA);
	printf("out:%s\n", outBuffer);

	LPA_BCD_freeNumber(&testNumber);
	LPA_BCD_fromDecimalASCII(&testNumber, "100");
	LPA_BCD_toDecimalASCII(outBuffer, CHAR_BUFFER_SIZE, &testNumber);
	printf("Decimal '%s' outASCII:%s\n", "100", outBuffer);

	LPA_BCD_freeNumber(&testNumber);
	LPA_BCD_fromHexadecimalASCII(&testNumber, "100");
	LPA_BCD_toHexadecimalASCII(outBuffer, CHAR_BUFFER_SIZE, &testNumber);
	printf("Hex '%s' outASCII:%s\n", "100", outBuffer);

	for (i = 1; i < argc; ++i)
	{
		char outBuffer2[CHAR_BUFFER_SIZE];

		LPA_BCD_freeNumber(&testNumber);
		LPA_BCD_fromDecimalASCII(&testNumber, argv[i]);
		LPA_BCD_toDecimalASCII(outBuffer, CHAR_BUFFER_SIZE, &testNumber);
		LPA_BCD_toHexadecimalASCII(outBuffer2, CHAR_BUFFER_SIZE, &testNumber);
		printf("Decimal '%s' Decimal outASCII:%s Hex outASCII:%s\n", argv[i], outBuffer, outBuffer2);

		LPA_BCD_freeNumber(&testNumber);
		LPA_BCD_fromHexadecimalASCII(&testNumber, argv[i]);
		LPA_BCD_toHexadecimalASCII(outBuffer, CHAR_BUFFER_SIZE, &testNumber);
		LPA_BCD_toDecimalASCII(outBuffer2, CHAR_BUFFER_SIZE, &testNumber);
		printf("Hex '%s' Hex outASCII:%s Decimal outASCII:%s\n", argv[i], outBuffer, outBuffer2);
	}

	inA32 = (int)(inA);
	LPA_BCD_freeNumber(&testNumber);
	LPA_BCD_fromInt32(&testNumber, inA32);
	LPA_BCD_toDecimalASCII(outBuffer, CHAR_BUFFER_SIZE, &testNumber);
	printf("inA32:%d\n", inA32);
	printf("out:%s\n", outBuffer);

	LPA_BCD_freeNumber(&testNumber);
	LPA_BCD_freeNumber(&aNumber);
	LPA_BCD_freeNumber(&resultNumber);
	LPA_BCD_freeNumber(&tempNumber);

	if (doLPA_BCDTest1("+", LPA_BCD_add, inA, inB, 1) == 0)
	{
		return 0;
	}
	if (doLPA_BCDTest1("*", LPA_BCD_multiply, inA, inB, 1) == 0)
	{
		return 0;
	}
	if (doLPA_BCDTest1("-", LPA_BCD_subtract, inA, inB, 1) == 0)
	{
		return 0;
	}
	if (doLPA_BCDTest2("/", "%", LPA_BCD_divide, inA, inB, 1) == 0)
	{
		return 0;
	}

	if (doLPA_BCDSequentialTests(numTests) == 0)
	{
		fprintf(stderr, "#### Sequential Tests Failed ####\n");
		return 0;
	}
	printf("#### Passed ####\n");

	if (doLPA_BCDRandomTests(numTests) == 0)
	{
		fprintf(stderr, "#### Random Tests Failed ####\n");
		return 0;
	}
	printf("#### Passed ####\n");

	return 1;
}

static int testINT(const int argc, char** argv)
{
	LPA_INT_number testNumber;
	LPA_INT_number aNumber;
	LPA_INT_number bNumber;
	LPA_INT_number resultNumber;
	LPA_INT_number tempNumber;
	int inA32 = 166;
	unsigned long value = 0;
	long inA = 166;
	long inB = 166;
	long result = 0;
	long numTests = 1;
	char outBuffer[CHAR_BUFFER_SIZE];
	int i;

	LPA_INT_initNumber(&testNumber);
	LPA_INT_initNumber(&aNumber);
	LPA_INT_initNumber(&bNumber);
	LPA_INT_initNumber(&resultNumber);
	LPA_INT_initNumber(&tempNumber);
	if (argc > 1)
	{
		numTests = atol(argv[1]);
	}
	if (argc > 2)
	{
		sscanf(argv[2], "%lX", &value);
		inA = (long)value;
	}
	if (argc > 3)
	{
		sscanf(argv[3], "%lX", &value);
		inB = (long)value;
	}
	printf("################# INT Tests : %lu #################\n", numTests);

	printf("inA:0x%lX\n", inA);
	LPA_INT_fromInt64(&aNumber, inA);
	printf("inA:numDigits:%u\n", aNumber.numDigits);

	LPA_INT_toHexadecimalASCII(outBuffer, CHAR_BUFFER_SIZE, &aNumber);
	printf("inA:0x%lX\n", inA);
	printf("out:0x%s\n", outBuffer);

	printf("inB:0x%lX\n", inB);
	LPA_INT_fromInt64(&bNumber, inB);
	printf("inB:numDigits:%u\n", bNumber.numDigits);

	LPA_INT_toHexadecimalASCII(outBuffer, CHAR_BUFFER_SIZE, &bNumber);
	printf("inA:0x%lX\n", inA);
	printf("out:0x%s\n", outBuffer);

	LPA_INT_freeNumber(&testNumber);
	LPA_INT_fromHexadecimalASCII(&testNumber, "100");
	LPA_INT_toHexadecimalASCII(outBuffer, CHAR_BUFFER_SIZE, &testNumber);
	printf("'%s' outASCII:0x%s %d 0x%X\n", "100", outBuffer, testNumber.numDigits, testNumber.pDigits[0]);

	for (i = 1; i < argc; ++i)
	{
		char outBuffer2[CHAR_BUFFER_SIZE];

		LPA_INT_freeNumber(&testNumber);
		LPA_INT_fromDecimalASCII(&testNumber, argv[i]);
		LPA_INT_toDecimalASCII(outBuffer, CHAR_BUFFER_SIZE, &testNumber);
		LPA_INT_toHexadecimalASCII(outBuffer2, CHAR_BUFFER_SIZE, &testNumber);
		printf("in[%d] Hex '%s' Decimal outASCII:%s Hex outASCII:%s\n", i - 1, argv[i], outBuffer, outBuffer2);

		LPA_INT_freeNumber(&testNumber);
		LPA_INT_fromHexadecimalASCII(&testNumber, argv[i]);
		LPA_INT_toHexadecimalASCII(outBuffer, CHAR_BUFFER_SIZE, &testNumber);
		LPA_INT_toDecimalASCII(outBuffer2, CHAR_BUFFER_SIZE, &testNumber);
		printf("in[%d] Hex '%s' Hex outASCII:%s Decimal outASCII:%s\n", i - 1, argv[i], outBuffer, outBuffer2);
	}

	inA32 = (int)(inA);
	LPA_INT_freeNumber(&testNumber);
	LPA_INT_fromInt32(&testNumber, inA32);
	result = inA;
	LPA_INT_toHexadecimalASCII(outBuffer, CHAR_BUFFER_SIZE, &testNumber);
	printf("out a32:\t0x%s 0x%lX %d 0x%X\n", outBuffer, (unsigned long)result, testNumber.numDigits, testNumber.pDigits[0]);

	LPA_INT_freeNumber(&testNumber);
	LPA_INT_freeNumber(&aNumber);
	LPA_INT_freeNumber(&bNumber);
	LPA_INT_freeNumber(&resultNumber);
	LPA_INT_freeNumber(&tempNumber);

	if (doLPA_INTTest2("/", "%", LPA_INT_divide, inA, inB, 1) == 0)
	{
		return 0;
	}
	if (doLPA_INTTest2("*/:/", "*/:%", LPA_INT_multiplyDivide, inA, inB, 1) == 0)
	{
		return 0;
	}
	if (doLPA_INTTest1("+", LPA_INT_add, inA, inB, 1) == 0)
	{
		return 0;
	}
	if (doLPA_INTTest1("*", LPA_INT_multiply, inA, inB, 1) == 0)
	{
		return 0;
	}
	if (doLPA_INTTest1("-", LPA_INT_subtract, inA, inB, 1) == 0)
	{
		return 0;
	}

	if (doLPA_INTSequentialTests(numTests) == 0)
	{
		fprintf(stderr, "#### Sequential Tests Failed ####\n");
		return 0;
	}
	printf("#### Passed ####\n");

	if (doLPA_INTRandomTests(numTests) == 0)
	{
		fprintf(stderr, "#### Random Tests Failed ####\n");
		return 0;
	}
	printf("#### Passed ####\n");

	return 1;
}

int main(const int argc, char** argv)
{
	int i;
	for (i = 0; i < argc; i++)
	{
		printf("argv[%d] '%s'\n", i, argv[i]);
	}

	if (testINT(argc, argv) == 0)
	{
		fprintf(stderr, "### INT tests failed ###\n");
		return -1;
	}
	if (testBCD(argc, argv) == 0)
	{
		fprintf(stderr, "### BCD tests failed ###\n");
		return -1;
	}
	return 0;
}
