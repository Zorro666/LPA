/*
Binary coded decimal large precision arithmetic functions
*/

#include <stdlib.h>
#include <stdint.h>

typedef uint8_t LPA_uint8;
typedef uint32_t LPA_uint32;

void LPA_ERROR(const char* const);

/* Users of this library have to supply these functions */
void* LPA_allocMem(size_t numBytes);
void LPA_freeMem(void* pMem);

