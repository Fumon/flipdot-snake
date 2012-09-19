#include "fumem.h"

void fumemclear(void* dst, unsigned int size) {
	unsigned int i;
	for (i = 0; i < size; ++i) {
		*(unsigned char*)(dst + i) = 0x00;
	}
}

void fumemcpy(void* dst, void* src, unsigned int size) {
	unsigned int i;
	for (i = 0; i < size; ++i)
	{
		*(unsigned char*)(dst + i) = *(unsigned char*)(src + i);
	}
}