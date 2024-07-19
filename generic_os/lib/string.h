// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //
// *Supertos Industries
// Author: Supertos, 2024
// string.h library used in Generic OS
#pragma once

#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>

char* Copy( char* from, char* to, size_t n ); // Copies chunk of data to another location.
char* Reverse( char* s, size_t n ); // Reverses byte order.
char* Format( char* buf, char* format, ... ); // Formats string with given arguments.
size_t Len( char* s ); // Returns length of the string.
char* Find( char* trg, char* what, size_t n, size_t nn ); // Returns pointer to the first encounter of what in trg.
char* Scan( char* buf, char* restrict format, ... ); // Scans buf and fills variables based on format.