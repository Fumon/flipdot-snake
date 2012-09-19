#ifndef SIMPLERAND_H
#define SIMPLERAND_H
#include <stdint.h>

typedef uint32_t simplerand;

void srsrand(uint32_t seed);

uint32_t srrand();

#endif
