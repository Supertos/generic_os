/* ========================================================= 
	This particular code is stolen from StackOverflow
 ========================================================= */
#include "stdlib.h"

int atoi(const char *p) {
    int k = 0;
    while (*p) {
        k = (k << 3) + (k << 1) + (*p) - '0';
        p++;
     }
     return k;
}