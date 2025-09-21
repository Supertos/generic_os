/* Supertos Industries
	Generic OS Kernel Simple Atomary set/get
*/
#include "Atomic.h"

#define ZF (1ULL << 6)

void AtomicAnd64( uint64_t* trg, uint64_t mask ) {
	__asm__ __volatile__ (
		"lock andq %1, %0;"
		: "+m"(*trg) : "r"(mask)
		: "cc" : "memory"
	);
}


void AtomicAnd8( uint8_t* trg, uint8_t mask ) {
	__asm__ __volatile__ (
		"lock andb %1, %0;"
		: "+m"(*trg) : "r"(mask)
		: "cc" : "memory"
	);
}

void AtomicOr64( uint64_t* trg, uint64_t mask ) {
	__asm__ __volatile__ (
		"lock orq %1, %0;"
		: "+m"(*trg) : "r"(mask)
		: "cc" : "memory"
	);
}


void AtomicOr8( uint8_t* trg, uint8_t mask ) {
	__asm__ __volatile__ (
		"lock orb %1, %0;"
		: "+m"(*trg) : "r"(mask)
		: "cc" : "memory"
	);
}


void AtomicCompareAndExchange8( uint8_t* val, uint8_t set ) {
	uint8_t flags = 0;

	while( !(flags & ZF) ) {
		__asm__ __volatile__ (
			"cmpxchgb %%2, %%0;"
			"movq %RFLAGS, %%1;"
			: "+m"(*val), "+m"(flags)
			: "r"(set)
			: "cc"
		);
	}
}