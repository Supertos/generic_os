// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// *Supertos Industries
// Author: Supertos, 2024
// Interrupt Service Routine implementations. All *.c files in int folder compiled with -mgeneral-regs-only flag.
#include "../drv/vga.h"
#include "../lib/string.h"
#include <stdint.h>
struct GateFrame {
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
};

// __attribute__((__interrupt__)) void GenericExceptionHandler( struct GateFrame* frame );
// __attribute__((__interrupt__)) void INT0( struct GateFrame* frame );
// __attribute__((__interrupt__)) void INT1( struct GateFrame* frame );
// __attribute__((__interrupt__)) void INT2( struct GateFrame* frame );
// __attribute__((__interrupt__)) void INT3( struct GateFrame* frame );
// __attribute__((__interrupt__)) void INT4( struct GateFrame* frame );
// __attribute__((__interrupt__)) void INT5( struct GateFrame* frame );
__attribute__((__interrupt__)) void INT6( struct GateFrame* frame );
// __attribute__((__interrupt__)) void INT7( struct GateFrame* frame );
// __attribute__((__interrupt__)) void INT8( struct GateFrame* frame );
// __attribute__((__interrupt__)) void INT9( struct GateFrame* frame );
// __attribute__((__interrupt__)) void INT10( struct GateFrame* frame );
// __attribute__((__interrupt__)) void INT11( struct GateFrame* frame );
// __attribute__((__interrupt__)) void INT12( struct GateFrame* frame );
// __attribute__((__interrupt__)) void INT13( struct GateFrame* frame );
// __attribute__((__interrupt__)) void INT14( struct GateFrame* frame );
// __attribute__((__interrupt__)) void INT15( struct GateFrame* frame );
// __attribute__((__interrupt__)) void INT16( struct GateFrame* frame );
// __attribute__((__interrupt__)) void INT17( struct GateFrame* frame );
// __attribute__((__interrupt__)) void INT18( struct GateFrame* frame );
// __attribute__((__interrupt__)) void INT19( struct GateFrame* frame );
// __attribute__((__interrupt__)) void INT20( struct GateFrame* frame );
// __attribute__((__interrupt__)) void INT21( struct GateFrame* frame );
// __attribute__((__interrupt__)) void INT22( struct GateFrame* frame );
// __attribute__((__interrupt__)) void INT23( struct GateFrame* frame );
// __attribute__((__interrupt__)) void INT24( struct GateFrame* frame );
// __attribute__((__interrupt__)) void INT25( struct GateFrame* frame );
// __attribute__((__interrupt__)) void INT26( struct GateFrame* frame );
// __attribute__((__interrupt__)) void INT27( struct GateFrame* frame );
// __attribute__((__interrupt__)) void INT28( struct GateFrame* frame );
// __attribute__((__interrupt__)) void INT29( struct GateFrame* frame );
// __attribute__((__interrupt__)) void INT30( struct GateFrame* frame );
// __attribute__((__interrupt__)) void INT31( struct GateFrame* frame );