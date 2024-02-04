/* ========================================================= 
	Contains basic string manipulation
 ========================================================= */
#include "string.h"

char *strncat(char *dest, const char *src, size_t n) {
	size_t offset = 0;
	if( n > strlen(src) ) { n = strlen(src); }
	
	for( size_t i = 0; i < n; i++ ) {
		dest[ strlen(dest) + offset ] = src[ offset ];
		offset++;
	}
	dest[ offset ] = '\0';
	return dest;
}