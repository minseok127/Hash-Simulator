#include "types.h"

int DivIndexing(int bincount, void* out)
{
    return *(uint32_t*)out % bincount;
}
