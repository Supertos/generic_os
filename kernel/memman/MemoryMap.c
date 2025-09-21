/* Supertos Industries
	Memory Map utilities.
	Memory Map passed to KernelEntry from compatible loader.
*/
#include "MemoryMap.h"
#include "utils/Architecture.h"

#include <stdint.h>
#include <stddef.h>


/* Checks whether MemoryEntry is available for OS use */
bool IsMemoryEntryAvailable( MemoryEntry* entry ) {
	return !!(entry->Type & 1);
}


/* Returns next MemoryEntry within [begin, begin+length]. */
MemoryEntry* NextMemoryEntryWithin( uintptr_t begin, size_t length, MemoryEntry* map, MemoryEntry** cur, size_t count ) {
	uintptr_t end = begin + length;
	MemoryEntry* mapEnd = map + count;
	
	MemoryEntry* next = *cur ? *cur + 1 : map;
	for( ; next < mapEnd; ++next ) {
		if( !IsMemoryEntryAvailable(next) ) continue;
		
		uintptr_t entryEnd = next->Begin + next->Pages * PAGE_SIZE;
		if( next->Begin >= end || entryEnd <= begin ) continue;
			
		*cur = next;
		return next;
	}
	
	*cur = NULL;
	return NULL;
}