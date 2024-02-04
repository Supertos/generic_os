/* ========================================================= 
	Contains basic string manipulation
 ========================================================= */
#include "string.h"

char *strchr(const char *s, int c) {
	size_t size = strlen(s);
	
	for( size_t i = 0; i < size; i++ ) {
		if( s[i] == c ) {
			return (char*)(s + i);
		}
	}
	return NULL;
}