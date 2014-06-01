#include "lpa.h"
#include "stdio.h"

void LPA_ERROR(const char* const message)
{
	fprintf(stderr,"LPA_ERROR:'%s'", message);
}
