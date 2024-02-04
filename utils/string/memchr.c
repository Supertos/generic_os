/* ========================================================= 
	Contains basic string manipulation
 ========================================================= */
#include "string.h"

void *memchr(const char *s, char c, size_t n) {
	
	for( size_t i = 0; i < n; i++ ) {
		if( c == *(i+s) ){ return (void*)(i+s); }
	}
	
	return NULL;
}

