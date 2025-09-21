/* Supertos Industries
	Memory Map utilities.
	Memory Map passed to KernelEntry from compatible loader.
*/
#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct MemoryEntry MemoryEntry;
struct MemoryEntry {
	uintptr_t Begin;
	size_t Pages : 56;
	uint8_t Type : 8;
} __attribute__((__packed__));


/* Checks whether MemoryEntry is available for OS use */
bool IsMemoryEntryAvailable( MemoryEntry* entry );


/* Returns next MemoryEntry within [begin, begin+length]. */
MemoryEntry* NextMemoryEntryWithin( uintptr_t begin, size_t length, MemoryEntry* map, MemoryEntry** cur, size_t count );