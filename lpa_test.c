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
	unsigned int inNumber = 166;
	char outBuffer[2048];

	LPA_BCDinitNumber(&sumNumber);
	for (i = 0; i < argc; i++)
	{
		printf("argv[%d] '%s'\n", i, argv[i]);
	}
	if (argc > 1)
	{
		inNumber = (unsigned int)atol(argv[1]);
	}
	printf("inNumber:%u\n", inNumber);
	LPA_BCDcreateNumber(&testNumber, inNumber);
	printf("memorySize:%u\n", testNumber.memorySize);

	LPA_BCDsprintf(&testNumber, outBuffer, 2048);
	printf("in:%u\n", inNumber);
	printf("out:%s\n", outBuffer);

	LPA_BCDcreateNumber(&aNumber, 100);
	LPA_BCDcreateNumber(&bNumber, 200);
	LPA_BCDadd(&aNumber, &bNumber, &sumNumber);

	LPA_BCDsprintf(&aNumber, outBuffer, 2048);
	printf("a:%s\n", outBuffer);
	LPA_BCDsprintf(&bNumber, outBuffer, 2048);
	printf("b:%s\n", outBuffer);
	LPA_BCDsprintf(&sumNumber, outBuffer, 2048);
	printf("sum:%s\n", outBuffer);

	return -1;
}
