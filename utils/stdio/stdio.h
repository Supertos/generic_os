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

int printf(const char* restrict format, ...);

#ifdef __cplusplus
}
#endif