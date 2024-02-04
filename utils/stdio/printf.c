/* ========================================================= 
	This particular code is stolen from OsDev wiki and adapted
 ========================================================= */
#include "stdio.h"
#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include "../string/string.h"
#include "../stdlib/stdlib.h"
#include "../../drv/vga.h"
int printf(const char* restrict format, ...) {
	va_list parameters;
	va_start(parameters, format);
 
	char str[32];
 
	while (*format != '\0') {
 
		if (format[0] != '%' || format[1] == '%') {
			vga_putchar(*format);
			format++;
			continue;
		}
 
		const char* format_begun_at = format++;
 
		if (*format == 'c') {
			format++;
			char c = (char) va_arg(parameters, int /* char promotes to int */);
			vga_putchar(c);
		} else if (*format == 's') {
			format++;
			const char* str = va_arg(parameters, const char*);
			vga_print(str);
		} else if (*format == 'i') {
			format++;
			itoa(va_arg(parameters, int), str, 10);
			vga_print(str);
		} else if (*format == 'd') {
			format++;
			itoa(va_arg(parameters, double), str, 10);
			vga_print(str);
		} else if (*format == 'l') {
			format++;
			itoa(va_arg(parameters, long), str, 10);
			vga_print(str);
		} else if (*format == 'x') {
			format++;
			itoa(va_arg(parameters, long), str, 16);
			vga_print(str);
		} else {
			format = format_begun_at;
			size_t len = strlen(format);
			
			vga_putchar(*format);
			format += len;
		}
	}
 
	va_end(parameters);
	return 0;
}