// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// *Supertos Industries
// Author: Supertos, 2024
// Generic OS Kernel, Memory Manager

#include "lib/stack.h"
#include "lib/sort.h"
#include "defines.h"
#include <stddef.h>
#include <stdint.h>

#define MEM_MIN_PAGE MEM_KERNEL_RESERVE / MEM_PAGE_SIZE

typedef struct PageEntry PageEntry;
typedef struct MemoryMapEntry MemoryMapEntry;
typedef struct MemoryMap MemoryMap;

struct PageEntry {
	size_t start;
	size_t length;
} __attribute__((__packed__));

struct MemoryMapEntry {
	uint64_t base;
	uint64_t length;
	uint32_t type;
	uint32_t attr; // Generic Bootloader always dumps all 24 bytes
} __attribute__((__packed__));

struct MemoryMap {
	uint32_t count;
	uint32_t reserved;
	
	MemoryMapEntry contents[];
} __attribute__((__packed__));

struct PageTable {
	void* contents[1024];
} __attribute__((__packed__));

struct PageDirectoryTable {
	PageTable* contents[1024];
} __attribute__((__packed__));

void InitializeMemoryManager( void ); // Initializes Memory Manager and fills PageStack.
void* AllocatePage();	// Allocates random physical page. This physical page shall be mapped in PageTable to be present in RAM.
void FreePage( void* ptr ); // Frees page and places it back in PageStack.
size_t PageStackSize( void ); // Returns entry count in PageStack.
void DefragmentPageStack( void ); // Sorts and merges page frames in PageStack.

void MapVirtualPage( void* ptr, void* trg );
void UnmapVirtualPage( void* ptr );