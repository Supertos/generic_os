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

__attribute__((__interrupt__)) void PrintStringInterrupt( struct GateFrame* frame );
__attribute__((__interrupt__)) void DivisionByZeroHandler( struct GateFrame* frame );