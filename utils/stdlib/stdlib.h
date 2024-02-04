/* ===============================================================================
	*utils/stdlib/stdlib.h
	stdlib.h standard library
=============================================================================== */
#pragma once

#include <stddef.h>
// #include <sys/cdefs.h>

#ifdef __cplusplus
extern "C" {
#endif

int atoi (const char *c);
long atol (const char *c);
void* itoa(int input, char *buffer, int radix);
void crash_and_burn(const char *msg);
#ifdef __cplusplus
}
#endif