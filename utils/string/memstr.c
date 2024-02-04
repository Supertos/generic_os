/* ========================================================= 
	Contains basic string manipulation
 ========================================================= */
#include "string.h"

char* memstr(const char* data, size_t start, size_t end) {
	char* start_ptr = (char*)start;
	char* end_ptr = (char*)end;
	size_t index = 0;
	
	size_t size = strlen(data);
	
	for( char* counter = start_ptr; counter < end_ptr; counter++ ) {
		if( data[index++] == *(counter) ) {
			if( index == size ) { return counter - size + 1; }	
		}else{
			index = 0;
		}
	}
	return NULL;
}