/* ========================================================= 
	Contains basic string manipulation
 ========================================================= */
#include "string.h"

void *memcpy(void *dest, const void *src, size_t n) {
	uint32_t *d = dest;
	const uint32_t *s = src;
	for( size_t i = 0; i < n/4; i++ ) { // Moving longs
		*(d++) = *(s++);
	}
	
	uint16_t *d2 = (uint16_t*)d;
	const uint16_t *s2 = (uint16_t*)s;
	
	for( size_t i = 0; i < n%4; i++ ) { // Moving words
		*(d2++) = *(s2++);
	}
	
	uint8_t *d3 = (uint8_t*)d2;
	const uint8_t *s3 = (uint8_t*)s2;
	
	for( size_t i = 0; i < n%2; i++ ) { // Moving chars
		*(d3++) = *(s3++);
	}
	return dest;
}

