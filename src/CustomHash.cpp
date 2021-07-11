#include <types.h>

void CustomHash_32(const void* key, int len, uint32_t seed, void* out)
{
    *(uint32_t*)out = *(uint32_t*)key;
}
