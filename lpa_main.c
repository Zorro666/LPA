#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>

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

	for (i = 0; i < argc; i++)
	{
		printf("argv[%d] '%s'\n", i, argv[i]);
	}

	return -1;
}
