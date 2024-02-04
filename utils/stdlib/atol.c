/* ========================================================= 
	This particular code is stolen from StackOverflow
 ========================================================= */
#include "stdlib.h"

long atol(const char *p) {
    long k = 0;
    while (*p) {
        k = (k << 3) + (k << 1) + (*p) - '0';
        p++;
     }
     return k;
}