/*
Binary coded decimal large precision arithmetic functions
*/
#ifndef LPA_CORE_H
#define LPA_CORE_H

#include <stdlib.h>
#include <stdint.h>

#define LPA_DEBUG 0

#if LPA_DEBUG
#include <stdio.h>
#endif

typedef uint8_t LPA_uint8;
typedef uint32_t LPA_uint32;
typedef uint64_t LPA_uint64;

void LPA_ERROR(const char* const);

/* Users of this library have to supply these functions */
void* LPA_allocMem(size_t numBytes);
void LPA_freeMem(void* pMem);

#if LPA_DEBUG
#define LPA_LOG printf
#else
#define LPA_LOG if(0) printf
#endif

#endif
