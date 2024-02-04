/* ========================================================= 
	Contains basic string manipulation
 ========================================================= */
#include "string.h"

int strncmp(const char *a, const char *b, size_t n) {
	int balance = 0;
	
	if( n > strlen(a) ){ n = strlen(a); }
	if( n > strlen(b) ){ n = strlen(b); }
	for( size_t i = 0; i < n; i++ ) {
		balance += *(a++) - *(b++);
	}
	return balance;
}