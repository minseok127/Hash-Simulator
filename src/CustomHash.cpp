#include "../include/types.h"

void CustomHash_32(const void* key, int length, uint32_t, void* out)
{
    *(uint32_t*)out = *(uint32_t*)key;
}
