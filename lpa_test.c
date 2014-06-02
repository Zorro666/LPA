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

int main(const int argc, char** argv)
{
	int i;
	LPA_BCDnumber testNumber;
	LPA_BCDnumber aNumber;
	LPA_BCDnumber bNumber;
	LPA_BCDnumber sumNumber;
	unsigned int inA = 166;
	unsigned int inB = 166;
	char outBuffer[2048];

	LPA_BCDinitNumber(&sumNumber);
	for (i = 0; i < argc; i++)
	{
		printf("argv[%d] '%s'\n", i, argv[i]);
	}
	if (argc > 1)
	{
		inA = (unsigned int)atol(argv[1]);
	}
	if (argc > 2)
	{
		inB = (unsigned int)atol(argv[2]);
	}
	printf("inNumber:%u\n", inA);
	LPA_BCDcreateNumber(&testNumber, inA);
	printf("memorySize:%u\n", testNumber.memorySize);

	LPA_BCDsprintf(&testNumber, outBuffer, 2048);
	printf("in:%u\n", inA);
	printf("out:%s\n", outBuffer);

	LPA_BCDcreateNumber(&aNumber, inA);
	LPA_BCDcreateNumber(&bNumber, inB);
	LPA_BCDadd(&aNumber, &bNumber, &sumNumber);

	LPA_BCDsprintf(&aNumber, outBuffer, 2048);
	printf("a:%s\n", outBuffer);
	LPA_BCDsprintf(&bNumber, outBuffer, 2048);
	printf("b:%s\n", outBuffer);
	LPA_BCDsprintf(&sumNumber, outBuffer, 2048);
	printf("sum:%s %u\n", outBuffer, inA+inB);

	return -1;
}
