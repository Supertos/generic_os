// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //
// *Supertos Industries
// Author: Supertos, 2024
// asm.h library used in Generic OS
#include "asm.h"

// PENTIUM INL instruction
uint32_t PortInDword( uint16_t id ) {
	uint32_t temp = 0;
	__asm__ __volatile__("movw %1, %%dx;in %%dx, %%eax;movl %%eax, %0;"
		:"=rm"(temp)
		:"rm"(id)
		: "eax", "dx"
    );
	return temp;
}

// PENTIUM OUTL instruction
void PortOutDword( uint16_t id, uint32_t value ) {
	__asm__ __volatile__("movw %0, %%dx;movl %1, %%eax;outl %%eax, %%dx;"
		:
		:"rm"(id), "rm"(value)
		:"eax", "dx"
	);
}

// PENTIUM OUTB instruction
void PortOutByte( uint16_t id, uint8_t value ) {
	__asm__ __volatile__("movw %0, %%dx;mov %1, %%al;outb %%al, %%dx;"
		:
		:"rm"(id), "rm"((uint8_t)value)
		:"al", "dx"
	);
}
// PENTIUM INB instruction
uint8_t PortInByte( uint16_t id ) {
	uint8_t temp = 0;
	__asm__ __volatile__("movw %1, %%dx;in %%dx, %%al;mov %%al, %0;"
		:"=rm"(temp)
		:"rm"(id)
		: "al", "dx", "memory"
    );
	return temp;
}