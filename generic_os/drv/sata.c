// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// *Supertos Industries
// Author: Supertos, 2024
// Version: 2
// VGA Support

#include "vga.h"


uint8_t vgaX = 0;
uint8_t vgaY = 0;
uint8_t vgaColor = 0x0F;

static inline volatile uint16_t* XYToVgaAddr( uint8_t x, uint8_t y ) { return (uint16_t*)(VGA_BUFFER + (size_t)( ( y % VGA_HEIGHT ) * VGA_WIDTH + x ) * 2); }

void MoveCursor( uint8_t x, uint8_t y ) {
	uint16_t pos = y * VGA_WIDTH + x;
	
	PortOutByte(0x3D4, (uint8_t)0x0F);
	PortOutByte(0x3D5, (uint8_t)(pos & 0xFF));
	PortOutByte(0x3D4, (uint8_t)0x0E);
	PortOutByte(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
	vgaX = x;
	vgaY = y;
}

void SetColor( uint8_t c ) {
	vgaColor = c;
}

void PrintChar( char c ) {
	switch( c ) {
		case '\n':
			vgaX = 0;
			vgaY = (vgaY + 1) % VGA_HEIGHT;
			break;
		case '\b':
			vgaX--;
			(*XYToVgaAddr( vgaX, vgaY )) = ( vgaColor << 8 ) | ' '; // First byte - ASCII char, second - color (0x0F for pure white)
			break;
		default:
			(*XYToVgaAddr( vgaX, vgaY )) = ( vgaColor << 8 ) | c; // First byte - ASCII char, second - color (0x0F for pure white)
			vgaX++;
			if( vgaX >= VGA_WIDTH ) { vgaX = 0; vgaY++; }
			break;
	}
	MoveCursor( vgaX, vgaY );
}

void PrintString( char* c ) {
	while( *(c) != '\0' ) {
		PrintChar( *(c) );
		c++;
	}
}