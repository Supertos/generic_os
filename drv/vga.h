/* ===============================================================================
	*drv/vga.h
	Contains basic functionality for VGA device
=============================================================================== */
#pragma once

#ifndef VGA_INCLUDED
#define VGA_INCLUDED
#include <stdint.h>
#include <stddef.h>

#include <stdbool.h>



void vga_setchar(size_t x, size_t y, uint16_t data);

void vga_enablecursor(uint8_t cursor_start, uint8_t cursor_end);

void vga_setcursor(int x, int y);

void vga_disablecursor(void);

void vga_clear(void);

void vga_enable();

void vga_setcolumn(size_t column);

void vga_setrow(size_t row);

void vga_print(const char* str);

void vga_putchar(const char chr);

#endif