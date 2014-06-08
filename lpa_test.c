#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

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

void printLPATestResult(const char* const name, const char* const lpaResult, long a, const char* const op, long b, long spResult)
{
	printf("%s:\t0x%s 0x%lX %s 0x%lX = 0x%lX\n", name, lpaResult, a, op, b, spResult);
}

typedef void(LPA_BCD_func)(const LPA_BCD_number* const pA, const LPA_BCD_number* const pB, LPA_BCD_number* const pResult);
typedef void(LPA_INT_func)(const LPA_INT_number* const pA, const LPA_INT_number* const pB, LPA_INT_number* const pResult);

int doLPATest(const char* const test, LPA_INT_func testFunc, const long a, const long b)
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
	return success;
}

int doLPARandomTests(const int numTests)
{
	int success = 1;
	int i = 0;

	while ((i < numTests) && (success == 1))
	{
		long a = rand();
		long b = rand();
		
		success = doLPATest("+", LPA_INT_add, a, b);
		if (b > a)
		{
			long temp = a;
			a = b;
			b = temp;
		}
		success = doLPATest("-", LPA_INT_subtract, a, b);
		++i;
	}

	return success;
}

int testBCD(const int argc, char** argv)
{
	LPA_BCD_number testNumber;
	LPA_BCD_number aNumber;
	LPA_BCD_number bNumber;
	LPA_BCD_number resultNumber;
	LPA_BCD_number tempNumber;
	int inA32 = 166;
	long inA = 166;
	long inB = 166;
	long temp = 0;
	long result = 0;
	char outBuffer[CHAR_BUFFER_SIZE];

	printf("################# BCD Tests #################\n");
	LPA_BCD_initNumber(&resultNumber);
	LPA_BCD_initNumber(&tempNumber);
	if (argc > 1)
	{
		inA = atol(argv[1]);
	}
	if (argc > 2)
	{
		inB = atol(argv[2]);
	}
	printf("inNumber:%ld\n", inA);
	LPA_BCD_fromInt64(&aNumber, inA);
	printf("numDigits:%u\n", aNumber.numDigits);

	LPA_BCD_toDecimalASCII(&aNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("in:%ld\n", inA);
	printf("out:%s\n", outBuffer);

	LPA_BCD_fromDecimalASCII(&testNumber, "100");
	LPA_BCD_toDecimalASCII(&testNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("'%s' outASCII:%s\n", "100", outBuffer);

	if (argc > 1)
	{
		LPA_BCD_fromDecimalASCII(&testNumber, argv[1]);
		LPA_BCD_toDecimalASCII(&testNumber, outBuffer, CHAR_BUFFER_SIZE);
		printf("'%s' outASCII:%s\n", argv[1], outBuffer);
	}
	if (argc > 2)
	{
		LPA_BCD_fromDecimalASCII(&testNumber, argv[2]);
		LPA_BCD_toDecimalASCII(&testNumber, outBuffer, CHAR_BUFFER_SIZE);
		printf("'%s' outASCII:%s\n", argv[2], outBuffer);
	}

	inA32 = (int)(inA);
	LPA_BCD_fromInt32(&testNumber, inA32);
	LPA_BCD_toDecimalASCII(&testNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("inA32:%d\n", inA32);
	printf("out:%s\n", outBuffer);

	LPA_BCD_fromInt64(&aNumber, inA);
	LPA_BCD_toDecimalASCII(&aNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("a:%s\n", outBuffer);

	LPA_BCD_fromInt64(&bNumber, inB);
	LPA_BCD_toDecimalASCII(&bNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("b:%s\n", outBuffer);

	LPA_BCD_add(&aNumber, &bNumber, &resultNumber);
	temp = result;
	result = inA + inB;
	LPA_BCD_toDecimalASCII(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("addAB:%s %ld + %ld = %ld\n", outBuffer, inA, inB, result);

	LPA_BCD_subtract(&aNumber, &bNumber, &resultNumber);
	temp = result;
	result = inA - inB;
	LPA_BCD_toDecimalASCII(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("subAB:%s %ld - %ld = %ld\n", outBuffer, inA, inB, result);
	temp = result;

	LPA_BCD_add(&aNumber, &resultNumber, &resultNumber);
	temp = result;
	result = inA + temp;
	LPA_BCD_toDecimalASCII(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("add:%s %ld + %ld = %ld\n", outBuffer, inA, temp, result);

	LPA_BCD_add(&resultNumber, &resultNumber, &resultNumber);
	temp = result;
	result = temp + temp;
	LPA_BCD_toDecimalASCII(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("add:%s %ld + %ld = %ld\n", outBuffer, temp, temp, result);

	LPA_BCD_add(&resultNumber, &bNumber, &resultNumber);
	temp = result;
	result = temp + inB;
	LPA_BCD_toDecimalASCII(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("add:%s %ld + %ld = %ld\n", outBuffer, temp, inB, result);

	LPA_BCD_add(&resultNumber, &bNumber, &resultNumber);
	temp = result;
	result = temp + inB;
	LPA_BCD_toDecimalASCII(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("add:%s %ld + %ld = %ld\n", outBuffer, temp, inB, result);

	LPA_BCD_subtract(&resultNumber, &bNumber, &resultNumber);
	temp = result;
	result = temp - inB;
	LPA_BCD_toDecimalASCII(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("sub:%s %ld - %ld = %ld\n", outBuffer, temp, inB, result);

	LPA_BCD_subtract(&resultNumber, &bNumber, &resultNumber);
	temp = result;
	result = temp - inB;
	LPA_BCD_toDecimalASCII(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("sub:%s %ld - %ld = %ld\n", outBuffer, temp, inB, result);

	LPA_BCD_subtract(&aNumber, &resultNumber, &resultNumber);
	temp = result;
	result = inA - temp;
	LPA_BCD_toDecimalASCII(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("sub:%s %ld - %ld = %ld\n", outBuffer, inA, temp, result);

	LPA_BCD_subtract(&bNumber, &resultNumber, &resultNumber);
	temp = result;
	result = inB - temp;
	LPA_BCD_toDecimalASCII(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("sub:%s %ld - %ld = %ld\n", outBuffer, inB, temp, result);

	LPA_BCD_subtract(&resultNumber, &resultNumber, &resultNumber);
	temp = result;
	result = temp - temp;
	LPA_BCD_toDecimalASCII(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("sub:%s %ld - %ld = %ld\n", outBuffer, temp, temp, result);

	return 1;
}

int testINT(const int argc, char** argv)
{
	LPA_INT_number testNumber;
	LPA_INT_number aNumber;
	LPA_INT_number bNumber;
	LPA_INT_number resultNumber;
	LPA_INT_number tempNumber;
	int inA32 = 166;
	long inA = 166;
	long inB = 166;
	long temp = 0;
	long result = 0;
	char outBuffer[CHAR_BUFFER_SIZE];

	printf("################# INT Tests #################\n");
	
	LPA_INT_initNumber(&resultNumber);
	LPA_INT_initNumber(&tempNumber);
	if (argc > 1)
	{
		inA = atol(argv[1]);
	}
	if (argc > 2)
	{
		inB = atol(argv[2]);
	}
	printf("inNumber:%ld\n", inA);
	LPA_INT_fromInt64(&aNumber, inA);
	printf("numDigits:%u\n", aNumber.numDigits);

	LPA_INT_toHexadecimalASCII(&aNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("in:%ld\n", inA);
	printf("out:0x%s\n", outBuffer);

	LPA_INT_fromHexadecimalASCII(&testNumber, "100");
	LPA_INT_toHexadecimalASCII(&testNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("'%s' outASCII:0x%s %d 0x%X\n", "100", outBuffer, testNumber.numDigits, testNumber.pDigits[0]);

	if (argc > 1)
	{
		LPA_INT_fromHexadecimalASCII(&testNumber, argv[1]);
		LPA_INT_toHexadecimalASCII(&testNumber, outBuffer, CHAR_BUFFER_SIZE);
		printf("'%s' %ld 0x%lX outASCII:0x%s %d 0x%X\n", argv[1], inA, (unsigned long)inA, outBuffer, testNumber.numDigits, testNumber.pDigits[0]);
	}
	if (argc > 2)
	{
		LPA_INT_fromHexadecimalASCII(&testNumber, argv[2]);
		LPA_INT_toHexadecimalASCII(&testNumber, outBuffer, CHAR_BUFFER_SIZE);
		printf("'%s' %ld 0x%lX outASCII:0x%s %d 0x%X\n", argv[2], inB, (unsigned long)inB, outBuffer, testNumber.numDigits, testNumber.pDigits[0]);
	}

	inA32 = (int)(inA);
	LPA_INT_fromInt32(&testNumber, inA32);
	result = inA;
	LPA_INT_toHexadecimalASCII(&testNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("out a32:\t0x%s 0x%lX %d 0x%X\n", outBuffer, (unsigned long)result, testNumber.numDigits, testNumber.pDigits[0]);

	LPA_INT_fromInt64(&aNumber, inA);
	result = inA;
	LPA_INT_toHexadecimalASCII(&aNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("out a:\t0x%s 0x%lX %d 0x%X\n", outBuffer, (unsigned long)result, aNumber.numDigits, aNumber.pDigits[0]);

	LPA_INT_fromInt64(&bNumber, inB);
	result = inB;
	LPA_INT_toHexadecimalASCII(&bNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("out b:\t0x%s 0x%lX %d 0x%X\n", outBuffer, (unsigned long)result, bNumber.numDigits, bNumber.pDigits[0]);

	LPA_INT_add(&aNumber, &bNumber, &resultNumber);
	temp = result;
	result = inA + inB;
	LPA_INT_toHexadecimalASCII(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printLPATestResult("addAB", outBuffer, inA, "+", inB, result);

	LPA_INT_add(&aNumber, &resultNumber, &resultNumber);
	temp = result;
	result = inA + temp;
	LPA_INT_toHexadecimalASCII(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printLPATestResult("addABA", outBuffer, inA, "+", temp, result);

	LPA_INT_subtract(&aNumber, &bNumber, &resultNumber);
	temp = result;
	result = inA - inB;
	LPA_INT_toHexadecimalASCII(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printLPATestResult("subAB", outBuffer, inA, "-", inB, result);

	LPA_INT_add(&aNumber, &resultNumber, &resultNumber);
	temp = result;
	result = inA + temp;
	LPA_INT_toHexadecimalASCII(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printLPATestResult("add", outBuffer, inA, "+", temp, result);

	LPA_INT_add(&resultNumber, &resultNumber, &resultNumber);
	temp = result;
	result = temp + temp;
	LPA_INT_toHexadecimalASCII(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printLPATestResult("add", outBuffer, temp, "+", temp, result);

	LPA_INT_add(&resultNumber, &bNumber, &resultNumber);
	temp = result;
	result = temp + inB;
	LPA_INT_toHexadecimalASCII(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printLPATestResult("add", outBuffer, temp, "+", inB, result);

	LPA_INT_add(&resultNumber, &bNumber, &resultNumber);
	temp = result;
	result = temp + inB;
	LPA_INT_toHexadecimalASCII(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printLPATestResult("add", outBuffer, temp, "+", inB, result);

	LPA_INT_subtract(&resultNumber, &bNumber, &resultNumber);
	temp = result;
	result = temp - inB;
	LPA_INT_toHexadecimalASCII(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printLPATestResult("sub", outBuffer, temp, "-", inB, result);

	LPA_INT_subtract(&resultNumber, &bNumber, &resultNumber);
	temp = result;
	result = temp - inB;
	LPA_INT_toHexadecimalASCII(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printLPATestResult("sub", outBuffer, temp, "-", inB, result);

	LPA_INT_subtract(&aNumber, &resultNumber, &resultNumber);
	temp = result;
	result = inA - temp;
	LPA_INT_toHexadecimalASCII(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printLPATestResult("sub", outBuffer, inA, "-", temp, result);

	LPA_INT_subtract(&bNumber, &resultNumber, &resultNumber);
	temp = result;
	result = inB - temp;
	LPA_INT_toHexadecimalASCII(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printLPATestResult("sub", outBuffer, inB, "-", temp, result);

	LPA_INT_subtract(&resultNumber, &resultNumber, &resultNumber);
	temp = result;
	result = temp - temp;
	LPA_INT_toHexadecimalASCII(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printLPATestResult("sub", outBuffer, temp, "-", temp, result);

	doLPATest("+", LPA_INT_add, inA, inB);

	doLPARandomTests(1000000);
	return 1;
}


int main(const int argc, char** argv)
{
	int i;
	for (i = 0; i < argc; i++)
	{
		printf("argv[%d] '%s'\n", i, argv[i]);
	}

	if (testBCD(argc, argv) == 0)
	{
		fprintf(stderr, "BCD tests failed");
		return -1;
	}
	if (testINT(argc, argv) == 0)
	{
		fprintf(stderr, "BCD tests failed");
		return -1;
	}
	return 0;
}
