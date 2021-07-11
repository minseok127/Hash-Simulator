#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

typedef struct hashcode128
{
    uint32_t h1;
    uint32_t h2;
    uint32_t h3;
    uint32_t h4;
} uint128_t;

#endif // TYPES_H
