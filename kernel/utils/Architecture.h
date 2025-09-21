/* Supertos Industries
	Generic OS Kernel Architecture-related defines and code
*/
#pragma once

#include <stdint.h>

#define PAGE_SIZE (size_t)(4096)
#define PAGES_PER_HUGEPAGE (size_t)(512)
#define HUGEPAGE_SIZE PAGES_PER_HUGEPAGE * PAGE_SIZE

#define CACHELINE_LENGTH (size_t)(64)

#define MAX_CPUS 64
#define MAX_NUMAS 64

#define NULL (void*)(0)

uint32_t CPUAPICID();

uint64_t BSF( uint64_t input );

uint64_t BSR( uint64_t input );