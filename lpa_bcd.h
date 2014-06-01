/*
Binary coded decimal large precision arithmetic functions
*/

#include <stdlib.h>

/* Users of this library have to supply these functions */
void* LPA_allocMem(size_t numBytes);
void LPA_freeMem(void* pMem);
