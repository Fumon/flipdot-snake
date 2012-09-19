#include "simplerand.h"

static simplerand internalrand = 1;

void srsrand(uint32_t seed) {
  internalrand = seed;
}

uint32_t srrand() {
  internalrand = (internalrand >> 1) ^ (-(internalrand & 1u) & 0xD0000001u);
  return internalrand;
}
