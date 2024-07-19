// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// *Supertos Industries
// Author: Supertos, 2024
// Interrupt Service Routine implementations. All *.c files in int folder compiled with -mgeneral-regs-only flag.
#include "int.h"
#include "../lib/string.h"
#include "../lib/kcub.h"

__attribute__((__interrupt__)) void PrintStringInterrupt( struct GateFrame* frame  ) {
	
	PrintString( "Test, Nya!" );
	return;
}

__attribute__((__interrupt__)) void GenericExceptionHandler( struct GateFrame* frame  ) {
	CrashUndBurn( "Your machine has experienced an unrecoverable existential crisis.", 32 );
}