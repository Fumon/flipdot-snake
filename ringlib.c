#include "ringlib.h"

// 
int push(ring *r, void* newelement) {
	if(r->current_element_count >= r->nelem) {
		// Over capacity
		return 1;
	}
	r->current_element_count = r->current_element_count + 1;
	r->head = r->head + 1;

	memcpy(getaddress(r, 0), newelement, r->unitsize);

	return 0;
}

int pop(ring *r, void** output) {
	if(r->current_element_count <= 0){
		return 1;
	}

	r->current_element_count--;
	(*output) = getaddress(r, r->current_element_count);

	return 0;
}

int get(ring *r, int index, void** output) {
	void* tmp = NULL;

	if((tmp = getaddress(r, index)) == NULL) {
		// Addressing error
		return 1;
	}

	(*output) = tmp;
	return 0;
}

void* getaddress(ring *r, int index) {
	int newind = 0;
	if(index > r->current_element_count) {
		return NULL;
	}
	// Wraparound check
	newind = r->head - index;
	if(newind < 0) {
		newind = r->nelem + newind;
		if(newind < 0) {
			return NULL;
		}
	}

	return (r->memstore + (newind * r->unitsize));
}