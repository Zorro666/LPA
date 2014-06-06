#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>

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

int main(const int argc, char** argv)
{
	int i;
	LPA_BCDnumber testNumber;
	LPA_BCDnumber aNumber;
	LPA_BCDnumber bNumber;
	LPA_BCDnumber resultNumber;
	LPA_BCDnumber tempNumber;
	int inA32 = 166;
	long inA = 166;
	long inB = 166;
	long temp = 0;
	long result = 0;
	char outBuffer[CHAR_BUFFER_SIZE];

	LPA_BCDinitNumber(&resultNumber);
	LPA_BCDinitNumber(&tempNumber);
	for (i = 0; i < argc; i++)
	{
		printf("argv[%d] '%s'\n", i, argv[i]);
	}
	if (argc > 1)
	{
		inA = atol(argv[1]);
	}
	if (argc > 2)
	{
		inB = atol(argv[2]);
	}
	printf("inNumber:%ld\n", inA);
	LPA_BCDcreateNumberFromInt64(&aNumber, inA);
	printf("memorySize:%u\n", aNumber.memorySize);

	LPA_BCDcreateNumberFromASCII(&testNumber, "100");
	LPA_BCDsprintf(&aNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("in:%ld\n", inA);
	printf("out:%s\n", outBuffer);
	LPA_BCDsprintf(&testNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("outASCII:%s\n", outBuffer);

	inA32 = (int)(inA);
	LPA_BCDcreateNumberFromInt32(&testNumber, inA32);
	LPA_BCDsprintf(&testNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("inA32:%d\n", inA32);
	printf("out:%s\n", outBuffer);

	LPA_BCDcreateNumberFromInt64(&aNumber, inA);
	LPA_BCDsprintf(&aNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("a:%s\n", outBuffer);

	LPA_BCDcreateNumberFromInt64(&bNumber, inB);
	LPA_BCDsprintf(&bNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("b:%s\n", outBuffer);

	LPA_BCDadd(&aNumber, &bNumber, &resultNumber);
	temp = result;
	result = inA + inB;
	LPA_BCDsprintf(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("addAB:%s %ld + %ld = %ld\n", outBuffer, inA, inB, result);

	LPA_BCDsubtract(&aNumber, &bNumber, &resultNumber);
	temp = result;
	result = inA - inB;
	LPA_BCDsprintf(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("subAB:%s %ld - %ld = %ld\n", outBuffer, inA, inB, result);
	temp = result;

	LPA_BCDadd(&aNumber, &resultNumber, &resultNumber);
	temp = result;
	result = inA + temp;
	LPA_BCDsprintf(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("add:%s %ld + %ld = %ld\n", outBuffer, inA, temp, result);

	LPA_BCDadd(&resultNumber, &resultNumber, &resultNumber);
	temp = result;
	result = temp + temp;
	LPA_BCDsprintf(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("add:%s %ld + %ld = %ld\n", outBuffer, temp, temp, result);

	LPA_BCDadd(&resultNumber, &bNumber, &resultNumber);
	temp = result;
	result = temp + inB;
	LPA_BCDsprintf(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("add:%s %ld + %ld = %ld\n", outBuffer, temp, inB, result);

	LPA_BCDadd(&resultNumber, &bNumber, &resultNumber);
	temp = result;
	result = temp + inB;
	LPA_BCDsprintf(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("add:%s %ld + %ld = %ld\n", outBuffer, temp, inB, result);

	LPA_BCDsubtract(&resultNumber, &bNumber, &resultNumber);
	temp = result;
	result = temp - inB;
	LPA_BCDsprintf(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("sub:%s %ld - %ld = %ld\n", outBuffer, temp, inB, result);

	LPA_BCDsubtract(&resultNumber, &bNumber, &resultNumber);
	temp = result;
	result = temp - inB;
	LPA_BCDsprintf(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("sub:%s %ld - %ld = %ld\n", outBuffer, temp, inB, result);

	LPA_BCDsubtract(&aNumber, &resultNumber, &resultNumber);
	temp = result;
	result = inA - temp;
	LPA_BCDsprintf(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("sub:%s %ld - %ld = %ld\n", outBuffer, inA, temp, result);

	LPA_BCDsubtract(&bNumber, &resultNumber, &resultNumber);
	temp = result;
	result = inB - temp;
	LPA_BCDsprintf(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("sub:%s %ld - %ld = %ld\n", outBuffer, inB, temp, result);

	LPA_BCDsubtract(&resultNumber, &resultNumber, &resultNumber);
	temp = result;
	result = temp - temp;
	LPA_BCDsprintf(&resultNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("sub:%s %ld - %ld = %ld\n", outBuffer, temp, temp, result);

	return -1;
}
