/* ========================================================= 
	This particular code is stolen from Wikipedia and adopted
 ========================================================= */
#include "stdlib.h"

size_t strlen_rev(const char* str) {
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

void reverse(char s[])
{
    int i, j;
    char c;
 
    for (i = 0, j = strlen_rev(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}
 
void* itoa(int input, char *buffer, int radix) {
    int i, sign;
	char alp[37] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if((sign = input) < 0) 
        input = -input;
    i = 0;
    do {
        buffer[i++] = alp[input % radix];
    } while ((input /= radix) > 0);
    if (sign < 0)
        buffer[i++] = '-';
    buffer[i] = '\0';
    reverse(buffer);
	
	return buffer;
}