/* ===============================================================================
	*drv/vga.c
	Contains basic functionality for VGA device
=============================================================================== */

#include <stdint.h>
#include <stddef.h>

#include "vga.h"

#include "io.h"
#include "../utils/string/string.h"

#include <stdbool.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_BUFFERSTART 0xB8000

enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
	return fg | bg << 4;
}
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
	return (uint16_t) uc | (uint16_t) color << 8;
}
 
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;
bool vga_enabled = false;


void vga_setchar(size_t x, size_t y, uint16_t data) {
	if( !vga_enabled ){ return; }
	terminal_buffer[y * VGA_WIDTH + x] = data;
}

void vga_enablecursor(uint8_t cursor_start, uint8_t cursor_end) {
	out(0x3D4, 0x0A);
	out(0x3D5, (in(0x3D5) & 0xC0) | cursor_start);
 
	out(0x3D4, 0x0B);
	out(0x3D5, (in(0x3D5) & 0xE0) | cursor_end);
}

void vga_setcursor(int x, int y) {
	uint16_t pos = y * VGA_WIDTH + x;
 
	out(0x3D4, 0x0F);
	out(0x3D5, (uint8_t) (pos & 0xFF));
	out(0x3D4, 0x0E);
	out(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

void vga_disablecursor(void) {
	out(0x3D4, 0x0A);
	out(0x3D5, 0x20);
}

void vga_clear(void) {
	terminal_column = 0;
	terminal_row = 0;
	terminal_buffer = (uint16_t*) VGA_BUFFERSTART;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			vga_setchar( x, y, vga_entry(' ', terminal_color) );
		}
	}
}

void vga_enable() {
	vga_enabled = true;
}

void vga_setcolumn(size_t column) {
	terminal_column = column % VGA_WIDTH;
}

void vga_setrow(size_t row) {
	terminal_row = row % VGA_HEIGHT;
}

void vga_print(const char* str) {
	size_t size = strlen(str);
	
	for( size_t i = 0; i < size; i++ ) {
		switch( str[i] ) {
			case '\n':
				vga_setcolumn(0);
				vga_setrow(terminal_row + 1);
			break;
			case '\r':
				vga_setcolumn(0);
			break;
			case '\b':
				vga_setcolumn(terminal_column - 1);
				vga_setchar(terminal_column, terminal_row, vga_entry(' ', terminal_color));
			break;
			default:
				if( terminal_column >= VGA_WIDTH ) {
					vga_setcolumn(0);
					vga_setrow(terminal_row + 1);
				}
				vga_setchar(terminal_column, terminal_row, vga_entry(str[i], terminal_color));
			break;
		}
		vga_setcolumn( terminal_column + 1 );
	}
}

void vga_putchar(const char chr) {
	switch( chr ) {
		case '\n':
			vga_setcolumn(0);
			vga_setrow(terminal_row + 1);
		break;
		case '\r':
			vga_setcolumn(0);
		break;
		case '\b':
			vga_setcolumn(terminal_column - 1);
			vga_setchar(terminal_column, terminal_row, vga_entry(' ', terminal_color));
		break;
		default:
			if( terminal_column >= VGA_WIDTH ) {
				vga_setcolumn(0);
				vga_setrow(terminal_row + 1);
			}
			vga_setchar(terminal_column, terminal_row, vga_entry(chr, terminal_color));
			vga_setcolumn( terminal_column + 1 );
		break;
	}
}


