// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// *Supertos Industries
// Author: Supertos, 2024
// Generic OS Bootloader Stage 2

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

void main( void ) {
	uint8_t symbol = 0;
	volatile uint8_t* vgaMem = (uint8_t*)0xB8000;
	
	*(vgaMem + 1) = 0x5c;
	
	while( true )
		*(vgaMem) = symbol++;
}