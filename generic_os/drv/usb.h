// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// *Supertos Industries
// Author: Supertos, 2024
// Version: 2
// VGA support

#include "../lib/asm.h"
#include <stddef.h>

#pragma once

#define VGA_BUFFER 0x0B8000 // For 0x0 - 0xA VGA Modes
#define VGA_HEIGHT 25
#define VGA_WIDTH 80

void MoveCursor( uint8_t x, uint8_t y );

void PrintChar( char c );
void SetColor( uint8_t c );

void PrintString( char* c );