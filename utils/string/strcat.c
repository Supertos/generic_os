/* ========================================================= 
	Contains basic string manipulation
 ========================================================= */
#include "string.h"

char *strcat(char *dest, const char *src) {
	size_t offset = 0;
	size_t amount = strlen(src);
	
	for( size_t i = 0; i < amount; i++ ) {
		dest[ strlen(dest) + offset ] = src[ offset ];
		offset++;
	}
	dest[ offset ] = '\0';
	return dest;
}