// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// *Supertos Industries
// Author: Supertos, 2024
// Interrupt Service Routine implementations. All *.c files in int folder compiled with -mgeneral-regs-only flag.
#include "int.h"
#include "../lib/string.h"
#include "../lib/kcub.h"
// __attribute__((__interrupt__)) void INT0( struct GateFrame* frame ) { CrashUndBurn( "ID: 0", 4 ); }
// __attribute__((__interrupt__)) void INT1( struct GateFrame* frame ) { CrashUndBurn( "ID: 1", 4 ); }
// __attribute__((__interrupt__)) void INT2( struct GateFrame* frame ) { CrashUndBurn( "ID: 2", 4 ); }
// __attribute__((__interrupt__)) void INT3( struct GateFrame* frame ) { CrashUndBurn( "ID: 3", 4 ); }
// __attribute__((__interrupt__)) void INT4( struct GateFrame* frame ) { CrashUndBurn( "ID: 4", 4 ); }
// __attribute__((__interrupt__)) void INT5( struct GateFrame* frame ) { CrashUndBurn( "ID: 5", 4 ); }

__attribute__((__interrupt__)) void INT6( struct GateFrame* frame ) { CrashUndBurn( "no.6 - Invalid Opcode.", 11 ); }

// __attribute__((__interrupt__)) void INT7( struct GateFrame* frame ) { CrashUndBurn( "ID: 7", 4 ); }
// __attribute__((__interrupt__)) void INT8( struct GateFrame* frame ) { CrashUndBurn( "ID: 8", 4 ); }
// __attribute__((__interrupt__)) void INT9( struct GateFrame* frame ) { CrashUndBurn( "ID: 9", 4 ); }
// __attribute__((__interrupt__)) void INT10( struct GateFrame* frame ) { CrashUndBurn( "ID: 10", 4 ); }
// __attribute__((__interrupt__)) void INT11( struct GateFrame* frame ) { CrashUndBurn( "ID: 11", 4 ); }
// __attribute__((__interrupt__)) void INT12( struct GateFrame* frame ) { CrashUndBurn( "ID: 12", 4 ); }
// __attribute__((__interrupt__)) void INT13( struct GateFrame* frame ) { CrashUndBurn( "ID: 13", 4 ); }
// __attribute__((__interrupt__)) void INT14( struct GateFrame* frame ) { CrashUndBurn( "ID: 14", 4 ); }
// __attribute__((__interrupt__)) void INT15( struct GateFrame* frame ) { CrashUndBurn( "ID: 15", 4 ); }
// __attribute__((__interrupt__)) void INT16( struct GateFrame* frame ) { CrashUndBurn( "ID: 16", 4 ); }
// __attribute__((__interrupt__)) void INT17( struct GateFrame* frame ) { CrashUndBurn( "ID: 17", 4 ); }
// __attribute__((__interrupt__)) void INT18( struct GateFrame* frame ) { CrashUndBurn( "ID: 18", 4 ); }
// __attribute__((__interrupt__)) void INT19( struct GateFrame* frame ) { CrashUndBurn( "ID: 19", 4 ); }
// __attribute__((__interrupt__)) void INT20( struct GateFrame* frame ) { CrashUndBurn( "ID: 20", 4 ); }
// __attribute__((__interrupt__)) void INT21( struct GateFrame* frame ) { CrashUndBurn( "ID: 21", 4 ); }
// __attribute__((__interrupt__)) void INT22( struct GateFrame* frame ) { CrashUndBurn( "ID: 22", 4 ); }
// __attribute__((__interrupt__)) void INT23( struct GateFrame* frame ) { CrashUndBurn( "ID: 23", 4 ); }
// __attribute__((__interrupt__)) void INT24( struct GateFrame* frame ) { CrashUndBurn( "ID: 24", 4 ); }
// __attribute__((__interrupt__)) void INT25( struct GateFrame* frame ) { CrashUndBurn( "ID: 25", 4 ); }
// __attribute__((__interrupt__)) void INT26( struct GateFrame* frame ) { CrashUndBurn( "ID: 26", 4 ); }
// __attribute__((__interrupt__)) void INT27( struct GateFrame* frame ) { CrashUndBurn( "ID: 27", 4 ); }
// __attribute__((__interrupt__)) void INT28( struct GateFrame* frame ) { CrashUndBurn( "ID: 28", 4 ); }
// __attribute__((__interrupt__)) void INT29( struct GateFrame* frame ) { CrashUndBurn( "ID: 29", 4 ); }
// __attribute__((__interrupt__)) void INT30( struct GateFrame* frame ) { CrashUndBurn( "ID: 30", 4 ); }
// __attribute__((__interrupt__)) void INT31( struct GateFrame* frame ) { CrashUndBurn( "ID: 31", 4 ); }