// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //
// *Supertos Industries
// Author: Supertos, 2024
// kcub.h library used in Generic OS
#include "kcub.h"

void CrashUndBurn( char* faulttxt, size_t n ) {
	MoveCursor( 0, 0 );
	SetColor( 0x1F );
	for( size_t i = VGA_HEIGHT * VGA_WIDTH; i > 0; i-- ) {
		PrintChar( ' ' );
	}
	uint8_t centerx = VGA_WIDTH * 0.5;
	uint8_t centery = VGA_HEIGHT * 0.5;
	
	MoveCursor( centerx - 11, centery );
	
	SetColor( 0x79 );
	PrintString( "Kernel Crash Und Burn!" );
	SetColor( 0x1F );
	
	MoveCursor( centerx - n, centery + 1 );
	PrintString( faulttxt );
	MoveCursor( centerx - 21, centery + 4 );
	PrintString( "Reset your machine and pray it'll boot up." );
	
	while( 1 );
}