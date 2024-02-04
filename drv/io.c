/* ========================================================= 
	*drv/io.c
	ASM commands ported to c
 ========================================================= */
#pragma once

#include <stdint.h>
#include "io.h"
#include <stddef.h>

// PENTIUM INL instruction
uint32_t inl( uint16_t id ) {
	uint32_t temp = 0;
	__asm__ __volatile__("movw %1, %%dx;in %%dx, %%eax;movl %%eax, %0;"
		:"=rm"(temp)
		:"rm"(id)
		: "eax", "dx"
    );
	return temp;
}

// PENTIUM OUTL instruction
void outl( uint16_t id, uint32_t value ) {
	__asm__ __volatile__("movw %0, %%dx;movl %1, %%eax;outl %%eax, %%dx;"
		:
		:"rm"(id), "rm"(value)
		:"eax", "dx"
	);
}

// PENTIUM OUT instruction
void out( uint16_t id, uint8_t value ) {
	__asm__ __volatile__("movw %0, %%dx;mov %1, %%eax;out %%eax, %%dx;"
		:
		:"rm"(id), "rm"((uint32_t)value)
		:"eax", "dx"
	);
}
// PENTIUM IN instruction
uint8_t in( uint16_t id ) {
	uint16_t temp = 0;
	__asm__ __volatile__("movw %1, %%dx;in %%dx, %%ax;movw %%ax, %0;"
		:"=rm"(temp)
		:"rm"(id)
		: "ah", "dx", "memory"
    );
	return temp;
}
