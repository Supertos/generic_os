// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //
// *Supertos Industries
// Author: Supertos, 2024
// idt.h library used in Generic OS

#include "idt.h"

static IDTRegister IDTR;

void HookInterrupt( uint8_t intid, void* func, uint8_t type, uint8_t dpl ) {
	GateDescriptor* gate = &(IDTR.offset->contents[ intid ]);
	gate->addrlow = (uint16_t)(size_t)func;
	gate->addrhigh = (uint16_t)( (size_t)func >> 16 );
	
	gate->dpl = dpl;
	gate->type = type;
	
	gate->reserved = 0;
	gate->selector = 0x08;

	gate->present = 1;
}

void SetIDT( IDT* addr ) {
	IDTR.offset = addr;
	IDTR.size = IDT_SIZE * sizeof( GateDescriptor ) - 1;
	__asm__ __volatile__ ( "movl %0, %%eax; lidt (%%eax);"
	:
	:"rm"(&IDTR)
	:"eax"
	);
}