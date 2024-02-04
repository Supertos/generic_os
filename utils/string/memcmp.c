/* ========================================================= 
	Contains basic string manipulation
 ========================================================= */
#include "string.h"

int memcmp(const char *s1, const char *s2, size_t n) {
	int balance = 0;
	
	for( size_t i = 0; i < n; i++ ) {
		balance += *(s1++) - *(s2++);
	}
	return balance;
}