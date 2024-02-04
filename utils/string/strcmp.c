/* ========================================================= 
	Contains basic string manipulation
 ========================================================= */
#include "string.h"

int strcmp(const char *a, const char *b) {
	int balance = 0;
	
	size_t lenA = strlen(a);
	size_t lenB = strlen(b);
	
	size_t maxLen = ( lenA > lenB ) ? lenA : lenB;
	
	for( size_t i = 0; i < maxLen; i++ ) {
		if( i < lenA ){ balance += *(a++); }
		if( i < lenB ){ balance -= *(b++); }
	}
	return balance;
}