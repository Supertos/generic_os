// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// *Supertos Industries
// Author: Supertos, 2024
// Generic OS Bootloader Stage 2

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "lib/idt.h"
#include "lib/kcub.h"
#include "int/int.h"
#include "defines.h"


void KernelEntry( void ) { 
	SetIDT( (IDT*)0x100000 );
	for( size_t i = 32; i > 0; i-- ) {
		HookInterrupt( i - 1, (void*)GenericExceptionHandler, 0xF, 0 );
	}
	HookInterrupt( 64, (void*)PrintStringInterrupt, 0xE, 0 );
	
	// __asm__ __volatile__ ( "int $64" );
	
	
	size_t* v = 0x950000;
	*v = 0;
	while( true );
}