/* ========================================================= 
	Contains basic string manipulation
 ========================================================= */
#include "string.h"

char *strrchr(const char *s, int c) {
	size_t size = strlen(s);
	
	for( size_t i = 0; i < size; i++ ) {
		if( s[size - i] == c ) {
			return (char*)(s + size - i);
		}
	}
	return NULL;
}