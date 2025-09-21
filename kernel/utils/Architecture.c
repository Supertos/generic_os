/* Supertos Industries
	Generic OS Kernel Architecture-related defines and code
*/
#include "Architecture.h"

#define FatalError() __asm__ ( "ud2;" : : : )

uint32_t CPUAPICID() {
	uint32_t apic = ~0;
	__asm__ __volatile__(
		"movl $1, %%eax;"
		"cpuid;"
		"shrl $24 %%ebx;"
		"movl %%ebx, %0;"
		: "=r" (apic) : "0" (apic)
		: "eax", "ebx", "ecx", "edx", "cc"
	);
	
	return apic;
}

uint64_t BSF( uint64_t input ) {
	if( !input ) return ~0;
	uint64_t pos = 0;
	__asm__ __volatile__(
		"bsf %0, %1;"
		: "=r"(pos) : "r"(input)
		: "cc" );
	
	return pos;
}


uint64_t BSR( uint64_t input ) {
	if( !input ) return ~0;
	uint64_t pos = 0;
	__asm__ __volatile__(
		"bsr %0, %1;"
		: "=r"(pos) : "r"(input)
		: "cc" );
	
	return pos;
}