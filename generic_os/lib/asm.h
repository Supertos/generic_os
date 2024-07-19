// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //
// *Supertos Industries
// Author: Supertos, 2024
// asm.h library used in Generic OS
#include <stdint.h>

void PortOutByte( uint16_t port, uint8_t data );
uint8_t PortInByte( uint16_t id );
uint32_t PortInDword( uint16_t id );
void PortOutDword( uint16_t id, uint32_t value );

#define Interrupt(id) { __asm__ __volatile__("int $id"); }