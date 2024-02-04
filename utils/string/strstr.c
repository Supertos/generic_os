/* ========================================================= 
	Contains basic string manipulation
 ========================================================= */
#include "string.h"

char* strstr(const char* where, const char* what) {
	size_t size = strlen(where);
	size_t index = 0;
	
	size_t size_test = strlen(what);
	
	for( size_t i = 0; i < size; i++ ) {
		if( what[index++] == *(where++) ) {
			if( index == size_test ) { return where - size_test + 1; } // IT IS FLAWED BY DESIGN
		}else{
			index = 0;
		}
	}
	return NULL;
}