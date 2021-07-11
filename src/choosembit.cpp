#include "../include/types.h"

int ChooseMbit(int bincount, void* out)
{
    int m = -1;
    while (bincount != 0) {
        bincount /= 2;
        m++;
    }

    return *(uint32_t*)out >> (32 - m);
}
