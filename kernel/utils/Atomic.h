/* Supertos Industries
	Generic OS Kernel Simple Atomary value
*/
#pragma once

#include <stdint.h>

#define AtomicExchange64(trg, set)                         		\
({                                                            \
    void* _old;                                               \
    __asm__ __volatile__ (                                    \
        "xchgq %1, %0;"                                      \
        : "=r"(_old), "+m"(*(trg))                            \
        : "0"(set)                                           \
        : "memory"                                           \
    );                                                        \
    _old;                                                     \
})


#define AtomicExchange8(trg, set)                         		\
({                                                            \
    void* _old;                                               \
    __asm__ __volatile__ (                                    \
        "xchgb %1, %0;"                                      \
        : "=r"(_old), "+m"(*(trg))                            \
        : "0"(set)                                           \
        : "memory"                                           \
    );                                                        \
    _old;                                                     \
})

#define AtomicLoad(from)                                     \
({                                                            \
    void* _val;                                               \
    __asm__ __volatile__ (                                    \
        "movq %1, %0;"                                       \
        "lfence;"                                            \
        : "=r"(_val)                                         \
        : "m"(*(from))                                       \
        : "memory"                                           \
    );                                                        \
    _val;                                                     \
})


#define Pause() __asm__ __volatile__ ( "pause;" ::: )

void AtomicAnd64( uint64_t* trg, uint64_t mask );


void AtomicAnd8( uint8_t* trg, uint8_t mask );


void AtomicOr64( uint64_t* trg, uint64_t mask );


void AtomicOr8( uint8_t* trg, uint8_t mask );