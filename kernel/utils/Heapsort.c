/* Supertos Industries
	Generic OS Heapsort implementation
*/

#include "Heapsort.h"
#include <stdint.h>
#include <stddef.h>

static inline void* Element( void* begin, size_t id, size_t elementSize ) {
	return (uint8_t*)begin + id * elementSize;
}


void Heapify( void* trg, size_t id, size_t elementSize, size_t count, bool (*cmp)(void*, void*, void*), void (*swap)(void*, void*), void* additionalParameter ) {
	size_t largest = id;
	size_t left = id * 2;
	size_t right = id * 2 + 1;
	
	if( left < count && cmp(Element(trg, largest, elementSize), Element(trg, left, elementSize), additionalParameter) ) largest = left;
	if( right < count && cmp(Element(trg, largest, elementSize), Element(trg, right, elementSize), additionalParameter) ) largest = right;
	
	if( largest != id ) {
		swap( Element(trg, id, elementSize), Element(trg, largest, elementSize) );
		Heapify( trg, largest, elementSize, count, cmp, swap, additionalParameter );
	}
}


void Heapsort( void* trg, size_t elementSize, size_t length, bool (*cmp)(void*, void*, void*), void (*swap)(void*, void*), void* additionalParameter ) {
	size_t count = length / elementSize;
	
	for( size_t i = 0; i < count / 2; ++i )
		Heapify( trg, i, elementSize, count, cmp, swap, additionalParameter );
	
	for( long long int i = count - 1; i >= 0; --i ) {
		swap( trg, Element(trg, i, elementSize) );
		Heapify( trg, 0, elementSize, i, cmp, swap, additionalParameter );
	}
}