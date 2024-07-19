// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //
// *Supertos Industries
// Author: Supertos, 2024
// idt.h library used in Generic OS
#pragma once

#include <stdint.h>
#include <stddef.h>
#include "../defines.h"

typedef struct IDTRegister IDTRegister;
typedef struct IDT IDT;
typedef struct GateDescriptor GateDescriptor;

struct GateDescriptor {
	uint16_t addrlow;        			// 0...15 bits of addr
	uint16_t selector;        			// 0x08, the only segment in Generic OS
	uint8_t  reserved;        			// Reserved, always 0
	unsigned int type 	: 	4; 			// 0x0F / 0x0E
	unsigned int res	:	1;			// Reserved
	unsigned int dpl 	:	2;			// CPU Privilege Levels that are allowed to call this interrupt
	unsigned int present:	1;			// Present bit
	uint16_t addrhigh;        			// 16..31 bits of addr
} __attribute__((__packed__));

struct IDT {
	GateDescriptor contents[IDT_SIZE];	// 255 elements max, but may contain less.
} __attribute__((__packed__));

struct IDTRegister {
	uint16_t size;						// element_count * sizeof(GateDescriptor) - 1
	IDT* offset;						// IDT addr
} __attribute__((__packed__));

void HookInterrupt( uint8_t intid, void* func, uint8_t type, uint8_t dpl ); // Hooks function with no arguments to given int id
void SetIDT( IDT* addr );	// Loads IDT at given address