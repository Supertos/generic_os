/* ========================================================= 
	Contains basic string manipulation
 ========================================================= */
#include "string.h"

// TODO: Creating buffer is slow, try buffering with longs and words first instead 
void *memmove(void *dest, const void *src, size_t n) {
	
	char buffer[n];
	
	for( size_t i = 0; i < n; i++ ) { // Creating buffer
		buffer[i] = *((char*)src++);
	}
	
	uint32_t *d = dest;
	uint32_t *s = (uint32_t*)(&buffer);
	for( size_t i = 0; i < n/4; i++ ) { // Moving longs
		*(d++) = *(s++);
	}
	
	uint16_t *d2 = (uint16_t*)d;
	uint16_t *s2 = (uint16_t*)s;
	
	for( size_t i = 0; i < n%4; i++ ) { // Moving words
		*(d2++) = *(s2++);
	}
	
	uint8_t *d3 = (uint8_t*)d2;
	uint8_t *s3 = (uint8_t*)s2;
	
	for( size_t i = 0; i < n%2; i++ ) { // Moving chars
		*(d3++) = *(s3++);
	}
	return dest;
}

