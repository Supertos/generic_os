// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// *Supertos Industries
// Author: Supertos, 2024
// Generic OS Kernel, Entry Point

#include "defines.h"

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include "lib/idt.h"
#include "lib/kcub.h"
#include "int/int.h"
#include "memory.h"

#include "lib/string.h"

void test( size_t n, ... );

void KernelEntry( void ) { 
	SetIDT( (IDT*)0x100000 );
	HookInterrupt( 6, (void*)INT6, 0xF, 0 );
	
	InitializeMemoryManager();
	
	while( true );
}

