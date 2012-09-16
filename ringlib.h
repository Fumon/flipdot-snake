#ifndef RINGLIB_H
#define RINGLIB_H

#include <stdlib.h>
//#include <string.h>

typedef struct {
	void* memstore;
	int nelem;
	int head;
	int tail;
	unsigned int unitsize;
	int current_element_count;
} ring;


int push(ring *r, void* newelement);
int pop(ring *r, void** output);
int get(ring *r, int index, void** output);
void* getaddress(ring *r, int index);

#endif