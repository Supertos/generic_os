/* ===============================================================================
	*utils/string/string.h
	string.h standard library
=============================================================================== */
#pragma once

#include <stddef.h>
#include <stdint.h>
// #include <sys/cdefs.h>

#ifdef __cplusplus
extern "C" {
#endif

void *memcpy(void *dest, const void *src, size_t n);

void *memmove(void *dest, const void *src, size_t n);

void *memchr(const char *s, char c, size_t n);

int memcmp(const char *s1, const char *s2, size_t n);

void *memset(char *dest, int z, size_t n);

char *strcat(char *dest, const char *src);

char *strncat(char *dest, const char *src, size_t n);

char *strchr(const char *s, int c);

char *strrchr(const char *s, int c);

int strcmp(const char *, const char *);

int strncmp(const char *, const char *, size_t);

// int strcoll(const char *, const char *); // TODO: Not Implemented

char *strcpy(char *toHere, const char *fromHere);

char *strncpy(char *toHere, const char *fromHere, size_t n);

// char *strerror(int);  // TODO: Not Implemented

size_t strlen(const char *);

// size_t strspn(const char *s, const char *accept);  // TODO: Not Implemented

// size_t strcspn(const char *s, const char *reject);  // TODO: Not Implemented

// char *strpbrk(const char *s, const char *accept); // TODO: Not Implemented

char *strstr(const char *haystack, const char *needle);

// char *strtok(char *, const char *);  // TODO: Not Implemented

// size_t strxfrm(char *dest, const char *src, size_t n) // TODO: Not Implemented

char* memstr(const char* data, size_t start, size_t end); // WARN: Custom Method
#ifdef __cplusplus
}
#endif