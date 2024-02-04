/* ========================================================= 
	Contains basic string manipulation
 ========================================================= */
#include "string.h"

void *memset(char *dest, int z, size_t n) {
	
	for( size_t i = 0; i < n; i++ ) {
		*(dest++) = (char)z;
	}
	
	return dest - n;
}

