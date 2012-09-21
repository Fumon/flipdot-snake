#include "simplerand.h"

static simplerand internalrand = 1;

void srsrand(uint32_t seed) {
  internalrand = seed;
}

uint32_t srrand() {
	unsigned char bit  = ((internalrand >> 0) ^ (internalrand >> 2) ^ (internalrand >> 3) ^ (internalrand >> 5) ) & 1;
	internalrand = (internalrand >> 1) | (bit << 15);
	return internalrand;
}
