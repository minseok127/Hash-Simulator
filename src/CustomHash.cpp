#include "../include/types.h"
#include <iostream>

void CustomHash_32(const void* key, int, uint32_t, void* out)
{
    uint32_t h2345 = *((uint32_t*)((uint8_t*)key + 2));

    *(uint32_t*)out = h2345;
}
