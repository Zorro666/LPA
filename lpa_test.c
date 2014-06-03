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
	LPA_BCDnumber sumNumber;
	unsigned int inA32 = 166;
	unsigned long inA = 166;
	unsigned long inB = 166;
	char outBuffer[CHAR_BUFFER_SIZE];

	LPA_BCDinitNumber(&sumNumber);
	for (i = 0; i < argc; i++)
	{
		printf("argv[%d] '%s'\n", i, argv[i]);
	}
	if (argc > 1)
	{
		inA = (unsigned long)atol(argv[1]);
	}
	if (argc > 2)
	{
		inB = (unsigned long)atol(argv[2]);
	}
	printf("inNumber:%lu\n", inA);
	LPA_BCDcreateNumberFromUint64(&aNumber, inA);
	printf("memorySize:%u\n", aNumber.memorySize);

	LPA_BCDcreateNumberFromASCII(&testNumber, "100");
	LPA_BCDsprintf(&aNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("in:%lu\n", inA);
	printf("out:%s\n", outBuffer);
	LPA_BCDsprintf(&testNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("outASCII:%s\n", outBuffer);

	inA32 = (unsigned int)(inA);
	LPA_BCDcreateNumberFromUint32(&testNumber, inA32);
	LPA_BCDsprintf(&testNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("inA32:%u\n", inA32);
	printf("out:%s\n", outBuffer);

	LPA_BCDcreateNumberFromUint64(&aNumber, inA);
	LPA_BCDcreateNumberFromUint64(&bNumber, inB);
	LPA_BCDadd(&aNumber, &bNumber, &sumNumber);

	LPA_BCDsprintf(&aNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("a:%s\n", outBuffer);
	LPA_BCDsprintf(&bNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("b:%s\n", outBuffer);
	LPA_BCDsprintf(&sumNumber, outBuffer, CHAR_BUFFER_SIZE);
	printf("sum:%s %ld\n", outBuffer, inA+inB);

	return -1;
}
