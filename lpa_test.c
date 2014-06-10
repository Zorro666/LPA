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

#define CHAR_BUFFER_SIZE (2048)

typedef void(LPA_BCD_func1)(LPA_BCD_number* const pResult, const LPA_BCD_number* const pA, const LPA_BCD_number* const pB);
typedef void(LPA_BCD_func2)(LPA_BCD_number* const pResult1, LPA_BCD_number* const pResult2, const LPA_BCD_number* const pA, const LPA_BCD_number* const pB);
typedef void(LPA_INT_func)(const LPA_INT_number* const pA, const LPA_INT_number* const pB, LPA_INT_number* const pResult);

static int doLPA_BCDTest1(const char* const test, LPA_BCD_func1 testFunc, const long a, const long b)
{
	LPA_BCD_number lpaResult;
	LPA_BCD_number lpaA;
	LPA_BCD_number lpaB;
	char resultTruth[CHAR_BUFFER_SIZE];
	char outResult[CHAR_BUFFER_SIZE];
	char outA[CHAR_BUFFER_SIZE];
	char outB[CHAR_BUFFER_SIZE];
	long result = 0;
	int success = 1;

	LPA_BCD_initNumber(&lpaResult);
	LPA_BCD_initNumber(&lpaA);
	LPA_BCD_initNumber(&lpaB);

	LPA_BCD_fromInt64(&lpaA, a);
	LPA_BCD_fromInt64(&lpaB, b);
	LPA_BCD_initNumber(&lpaResult);

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

	testFunc(&lpaResult, &lpaA, &lpaB);

	LPA_BCD_toDecimalASCII(outA, &lpaA, CHAR_BUFFER_SIZE);
	LPA_BCD_toDecimalASCII(outB, &lpaB, CHAR_BUFFER_SIZE);
	LPA_BCD_toDecimalASCII(outResult, &lpaResult, CHAR_BUFFER_SIZE);
	if (0)
	{
		printf("%s:\tBCD: %s long: %ld BCD: %s %s %s = %s long: %ld %s %ld = %ld\n", 
				test, outResult, result, outA, test, outB, outResult, a, test, b, result);
	}

	sprintf(resultTruth, "%ld", result);
	if (strcmp(resultTruth, outResult) != 0)
	{
		success = 0;
	}
	LPA_BCD_freeNumber(&lpaResult);
	LPA_BCD_fromInt64(&lpaResult, result);
	LPA_BCD_toDecimalASCII(resultTruth, &lpaResult, CHAR_BUFFER_SIZE);
	if (strcmp(resultTruth, outResult) != 0)
	{
		success = 0;
	}
	if (success == 0)
	{
		fprintf(stderr, "%s:\tBCD: %s long: %ld\nBCD: %s %s %s = %s\nlong: %ld %s %ld = %ld\n", 
				test, outResult, result, outA, test, outB, outResult, a, test, b, result);
		fprintf(stderr, "TEST FAILED '%s' : %ld %s %ld = %s != %s\n", test, a, test, b, resultTruth, outResult);
	}

	LPA_BCD_freeNumber(&lpaResult);
	LPA_BCD_freeNumber(&lpaA);
	LPA_BCD_freeNumber(&lpaB);
	return success;
}

static int doLPA_BCDTest2(const char* const test1, const char* const test2, LPA_BCD_func2 testFunc, const long a, const long b)
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

	LPA_BCD_fromInt64(&lpaA, a);
	LPA_BCD_fromInt64(&lpaB, b);

	if (testFunc == LPA_BCD_divide)
	{
		if (b == 0)
		{
			return 1;
		}
		result1 = a / b;
		result2 = a % b;
	}

	testFunc(&lpaResult1, &lpaResult2, &lpaA, &lpaB);

	LPA_BCD_toDecimalASCII(outA, &lpaA, CHAR_BUFFER_SIZE);
	LPA_BCD_toDecimalASCII(outB, &lpaB, CHAR_BUFFER_SIZE);
	LPA_BCD_toDecimalASCII(outResult1, &lpaResult1, CHAR_BUFFER_SIZE);
	LPA_BCD_toDecimalASCII(outResult2, &lpaResult2, CHAR_BUFFER_SIZE);
	if (0)
	{
		printf("%s:\tBCD: %s long: %ld BCD: %s %s %s = %s long: %ld %s %ld = %ld\n", 
				test1, outResult1, result1, outA, test1, outB, outResult1, a, test1, b, result1);
	}

	sprintf(result1Truth, "%ld", result1);
	if (strcmp(result1Truth, outResult1) != 0)
	{
		success1 = 0;
	}

	LPA_BCD_freeNumber(&lpaResult1);
	LPA_BCD_fromInt64(&lpaResult1, result1);
	LPA_BCD_toDecimalASCII(result1Truth, &lpaResult1, CHAR_BUFFER_SIZE);
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

	if (0)
	{
		printf("%s:\tBCD: %s long: %ld BCD: %s %s %s = %s long: %ld %s %ld = %ld\n", 
				test2, outResult2, result2, outA, test2, outB, outResult2, a, test2, b, result2);
	}
	sprintf(result2Truth, "%ld", result2);
	if (strcmp(result2Truth, outResult2) != 0)
	{
		success2 = 0;
	}

	LPA_BCD_freeNumber(&lpaResult2);
	LPA_BCD_fromInt64(&lpaResult2, result2);
	LPA_BCD_toDecimalASCII(result2Truth, &lpaResult2, CHAR_BUFFER_SIZE);
	if (strcmp(result2Truth, outResult2) != 0)
	{
		success2 = 0;
	}

	if (success2 == 0)
	{
		fprintf(stderr, "%s:\tBCD: %s long: %ld\nBCD: %s %s %s = %s\nlong: %ld %s %ld = %ld\n", 
				test2, outResult2, result2, outA, test1, outB, outResult2, a, test2, b, result2);
		fprintf(stderr, "TEST FAILED '%s' : %ld %s %ld = %s != %s\n", test2, a, test1, b, result2Truth, outResult2);
	}

	LPA_BCD_freeNumber(&lpaResult1);
	LPA_BCD_freeNumber(&lpaResult2);
	LPA_BCD_freeNumber(&lpaA);
	LPA_BCD_freeNumber(&lpaB);

	return success1 && success2;
}

static int doLPA_BCDTests(const long numTests, const int mode)
{
	int success = 1;
	long i = 0;
	long a = 0;
	long b = 0;
	long maxValue = 0;

	printf("numTests:%ld\n", numTests);
	maxValue = (long)sqrt((float)numTests);
	printf("maxValue:%ld\n", maxValue);

	while ((i < numTests) && (success == 1))
	{
		if (mode == 0)
		{
			a = rand() - (RAND_MAX >> 1);
			b = rand() - (RAND_MAX >> 1);
		}
		else if (mode == 0)
		{
			b++;
			if (b > maxValue)
			{
				a++;
				b = 0;
			}
		}
		
		success = doLPA_BCDTest1("+", LPA_BCD_add, a, b);
		if (success == 0)
		{
			break;
		}

		success = doLPA_BCDTest1("*", LPA_BCD_multiply, a, b);
		if (success == 0)
		{
			break;
		}

		success = doLPA_BCDTest1("-", LPA_BCD_subtract, a, b);
		if (success == 0)
		{
			break;
		}
		if (a < 0)
		{
			a = -a;
		}
		if (b < 0)
		{
			b = -b;
		}
		success = doLPA_BCDTest2("/", "%", LPA_BCD_divide, a, b);
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
	return doLPA_BCDTests(numTests, 0);
}

static int doLPA_BCDSequentialTests(const long numTests)
{
	return doLPA_BCDTests(numTests, 1);
}

static int doLPA_INTTest(const char* const test, LPA_INT_func testFunc, const long a, const long b)
{
	LPA_INT_number lpaResult;
	LPA_INT_number lpaA;
	LPA_INT_number lpaB;
	char resultTruth[CHAR_BUFFER_SIZE];
	char outResult[CHAR_BUFFER_SIZE];
	char outA[CHAR_BUFFER_SIZE];
	char outB[CHAR_BUFFER_SIZE];
	long result = 0;
	int success = 1;

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

	testFunc(&lpaA, &lpaB, &lpaResult);

	LPA_INT_toHexadecimalASCII(&lpaA, outA, CHAR_BUFFER_SIZE);
	LPA_INT_toHexadecimalASCII(&lpaB, outB, CHAR_BUFFER_SIZE);
	LPA_INT_toHexadecimalASCII(&lpaResult, outResult, CHAR_BUFFER_SIZE);
	if (0)
	{
		printf("%s:\tLPA: 0x%s long: 0x%lX LPA: 0x%s %s 0x%s = 0x%s long: 0x%lX %s 0x%lX = 0x%lX\n", 
				test, outResult, result, outA, test, outB, outResult, a, test, b, result);
	}

	sprintf(resultTruth, "%lX", result);
	if (strcmp(resultTruth, outResult) != 0)
	{
		success = 0;
	}
	LPA_INT_freeNumber(&lpaResult);
	LPA_INT_fromInt64(&lpaResult, result);
	LPA_INT_toHexadecimalASCII(&lpaResult, resultTruth, CHAR_BUFFER_SIZE);
	if (strcmp(resultTruth, outResult) != 0)
	{
		success = 0;
	}
	if (success == 0)
	{
		fprintf(stderr, "%s:\tLPA: 0x%s long: 0x%lX\nLPA: 0x%s %s 0x%s = 0x%s\nlong: 0x%lX %s 0x%lX = 0x%lX\n", 
				test, outResult, result, outA, test, outB, outResult, a, test, b, result);
		fprintf(stderr, "TEST FAILED '%s' : 0x%lX %s 0x%lX = 0x%s != 0x%s\n", test, a, test, b, resultTruth, outResult);
	}

	LPA_INT_freeNumber(&lpaResult);
	LPA_INT_freeNumber(&lpaA);
	LPA_INT_freeNumber(&lpaB);
	return success;
}

static int doLPA_INTRandomTests(const long numTests)
{
	int success = 1;
	long i = 0;

	while ((i < numTests) && (success == 1))
	{
		long a = rand();
		long b = rand();
		
		success = doLPA_INTTest("+", LPA_INT_add, a, b);
		if (success == 0)
		{
			break;
		}

		a = a % INT_MAX;
		b = b % INT_MAX;
		success = doLPA_INTTest("*", LPA_INT_multiply, a, b);
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
		success = doLPA_INTTest("-", LPA_INT_subtract, a, b);
		if (success == 0)
		{
			break;
		}

		++i;
	}

	return success;
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

	LPA_BCD_toDecimalASCII(outBuffer, &aNumber, CHAR_BUFFER_SIZE);
	printf("in:%ld\n", inA);
	printf("out:%s\n", outBuffer);

	LPA_BCD_fromDecimalASCII(&testNumber, "100");
	LPA_BCD_toDecimalASCII(outBuffer, &testNumber, CHAR_BUFFER_SIZE);
	printf("'%s' outASCII:%s\n", "100", outBuffer);

	if (argc > 1)
	{
		LPA_BCD_freeNumber(&testNumber);
		LPA_BCD_fromDecimalASCII(&testNumber, argv[1]);
		LPA_BCD_toDecimalASCII(outBuffer, &testNumber, CHAR_BUFFER_SIZE);
		printf("'%s' outASCII:%s\n", argv[1], outBuffer);
	}
	if (argc > 2)
	{
		LPA_BCD_freeNumber(&testNumber);
		LPA_BCD_fromDecimalASCII(&testNumber, argv[2]);
		LPA_BCD_toDecimalASCII(outBuffer, &testNumber, CHAR_BUFFER_SIZE);
		printf("'%s' outASCII:%s\n", argv[2], outBuffer);
	}

	inA32 = (int)(inA);
	LPA_BCD_freeNumber(&testNumber);
	LPA_BCD_fromInt32(&testNumber, inA32);
	LPA_BCD_toDecimalASCII(outBuffer, &testNumber, CHAR_BUFFER_SIZE);
	printf("inA32:%d\n", inA32);
	printf("out:%s\n", outBuffer);

	LPA_BCD_freeNumber(&testNumber);
	LPA_BCD_freeNumber(&aNumber);
	LPA_BCD_freeNumber(&resultNumber);
	LPA_BCD_freeNumber(&tempNumber);

	if (doLPA_BCDTest1("+", LPA_BCD_add, inA, inB) == 0)
	{
		return 0;
	}
	if (doLPA_BCDTest1("*", LPA_BCD_multiply, inA, inB) == 0)
	{
		return 0;
	}
	if (doLPA_BCDTest1("-", LPA_BCD_subtract, inA, inB) == 0)
	{
		return 0;
	}
	if (doLPA_BCDTest2("/", "%", LPA_BCD_divide, inA, inB) == 0)
	{
		return 0;
	}

	if (doLPA_BCDSequentialTests(numTests) == 0)
	{
		fprintf(stderr, "#### Sequential Tests Failed ####\n");
		return 0;
	}

	if (doLPA_BCDRandomTests(numTests) == 0)
	{
		fprintf(stderr, "#### Random Tests Failed ####\n");
		return 0;
	}

	return 1;
}

static int testINT(const int argc, char** argv)
{
	LPA_INT_number testNumber;
	LPA_INT_number aNumber;
	LPA_INT_number resultNumber;
	LPA_INT_number tempNumber;
	int inA32 = 166;
	long inA = 166;
	long inB = 166;
	long result = 0;
	long numTests = 1;
	char outBuffer[CHAR_BUFFER_SIZE];

	LPA_INT_initNumber(&testNumber);
	LPA_INT_initNumber(&aNumber);
	LPA_INT_initNumber(&resultNumber);
	LPA_INT_initNumber(&tempNumber);
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
	printf("################# INT Tests : %lu #################\n", numTests);

	printf("inNumber:%ld\n", inA);
	LPA_INT_fromInt64(&aNumber, inA);
	printf("numDigits:%u\n", aNumber.numDigits);

	LPA_INT_toHexadecimalASCII(&aNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("in:%ld\n", inA);
	printf("out:0x%s\n", outBuffer);

	LPA_INT_freeNumber(&testNumber);
	LPA_INT_fromHexadecimalASCII(&testNumber, "100");
	LPA_INT_toHexadecimalASCII(&testNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("'%s' outASCII:0x%s %d 0x%X\n", "100", outBuffer, testNumber.numDigits, testNumber.pDigits[0]);

	if (argc > 1)
	{
		LPA_INT_freeNumber(&testNumber);
		LPA_INT_fromHexadecimalASCII(&testNumber, argv[1]);
		LPA_INT_toHexadecimalASCII(&testNumber, outBuffer, CHAR_BUFFER_SIZE);
		printf("'%s' %ld 0x%lX outASCII:0x%s %d 0x%X\n", argv[1], inA, (unsigned long)inA, outBuffer, testNumber.numDigits, testNumber.pDigits[0]);
	}
	if (argc > 2)
	{
		LPA_INT_freeNumber(&testNumber);
		LPA_INT_fromHexadecimalASCII(&testNumber, argv[2]);
		LPA_INT_toHexadecimalASCII(&testNumber, outBuffer, CHAR_BUFFER_SIZE);
		printf("'%s' %ld 0x%lX outASCII:0x%s %d 0x%X\n", argv[2], inB, (unsigned long)inB, outBuffer, testNumber.numDigits, testNumber.pDigits[0]);
	}

	inA32 = (int)(inA);
	LPA_INT_freeNumber(&testNumber);
	LPA_INT_fromInt32(&testNumber, inA32);
	result = inA;
	LPA_INT_toHexadecimalASCII(&testNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("out a32:\t0x%s 0x%lX %d 0x%X\n", outBuffer, (unsigned long)result, testNumber.numDigits, testNumber.pDigits[0]);

	LPA_INT_freeNumber(&testNumber);
	LPA_INT_freeNumber(&aNumber);
	LPA_INT_freeNumber(&resultNumber);
	LPA_INT_freeNumber(&tempNumber);

	if (doLPA_INTTest("+", LPA_INT_add, inA, inB) == 0)
	{
		return 0;
	}
	if (doLPA_INTTest("*", LPA_INT_multiply, inA, inB) == 0)
	{
		return 0;
	}
	if (doLPA_INTTest("-", LPA_INT_subtract, inA, inB) == 0)
	{
		return 0;
	}

	return doLPA_INTRandomTests(numTests);
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
