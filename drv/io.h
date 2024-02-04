/* ========================================================= 
	*drv/io.h
	ASM commands ported to c
 ========================================================= */
#pragma once
#include <stddef.h>
#include <stdint.h>

// PENTIUM INL instruction
uint32_t inl( uint16_t id );

// PENTIUM OUTL instruction
void outl( uint16_t id, uint32_t value );

// PENTIUM OUT instruction
void out( uint16_t id, uint8_t value );
// PENTIUM IN instruction
uint8_t in( uint16_t id );