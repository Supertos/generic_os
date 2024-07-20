// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// *Supertos Industries
// Author: Supertos, 2024
// Generic OS Kernel, Memory Manager

#include "memory.h"

DEFINE_STACK(PageEntry, PageStack)

uint8_t PageEntryCompare( PageEntry* a, PageEntry* b ) {
	return a->start >= b->start;
}

void PageEntrySwap( PageEntry* a, PageEntry* b ) {
	PageEntry temp = { .start = a->start, .length = a->length };
	a->start = b->start;
	a->length = b->length;
	
	b->start = temp.start;
	b->length = temp.length;
}

DEFINE_SORT(PageEntry, SortPageStack)


PageStack* PAGESTACK = (PageStack*)MEM_STACK_ADDR;
void InitializeMemoryManager( void ) {
	size_t availableram = MEM_KERNEL_RESERVE - 0x100000 - IDT_SIZE * 8 - 0x100000; // Leave for kernel no less than sizeof all tables + 1MiB. Default is ~1.5MiBs
	PAGESTACK->totalsize = availableram / sizeof( PageEntry );
	PAGESTACK->size = 0;
	
	volatile MemoryMap* memmap = (MemoryMap*)MEM_MAP_ADDR;
	
	size_t totalMemory = 0;
	for( size_t i = 0; i < memmap->count; i++ ) {	// Determine size of RAM and available memory for us to give out to the user.
		size_t entryEnd = memmap->contents[i].base+memmap->contents[i].length;
		totalMemory = entryEnd > totalMemory ? entryEnd : totalMemory;
		
		if( memmap->contents[i].type == 1 && memmap->contents[i].base + memmap->contents[i].length >= MEM_KERNEL_RESERVE ) {
			PageEntry* frame = PageStackPush( PAGESTACK );
			frame->length = memmap->contents[i].length / MEM_PAGE_SIZE;
			frame->start = memmap->contents[i].base / MEM_PAGE_SIZE;
			
			if( frame->start < MEM_MIN_PAGE ) {
				frame->length -= MEM_MIN_PAGE - frame->start;
				frame->start = MEM_MIN_PAGE;
			}
		}
	}
	
}

void* AllocatePage() {
	PageEntry* page = PageStackPeek( PAGESTACK );
	
	page->length--;
	if( PageStackPeek( PAGESTACK )->length == 0 ){ PageStackPop( PAGESTACK ); }
	return (void*)((page->start++)*MEM_PAGE_SIZE);
}

void FreePage( void* ptr ) {
	size_t pageId = (size_t)ptr / MEM_PAGE_SIZE;
	PageStackPush( PAGESTACK );
	PageEntry* pg = PageStackPeek( PAGESTACK );
	
	pg->length = 1;
	pg->start = pageId;
	DefragmentPageStack();
}

size_t PageStackSize( void ) {
	return PAGESTACK->size;
}

void DefragmentPageStack( void ) {
	SortPageStack( PAGESTACK->contents, PAGESTACK->size );
	
	size_t lastdefragid = 0;
	size_t initialsize = PAGESTACK->size;
	for( size_t i = 1; i < initialsize; i++ ) {
		if( PAGESTACK->contents[lastdefragid].start + PAGESTACK->contents[lastdefragid].length == PAGESTACK->contents[i].start ) {
			PAGESTACK->contents[lastdefragid].length += PAGESTACK->contents[i].length;
			PAGESTACK->size--;
		}else{
			lastdefragid++;
			PAGESTACK->contents[lastdefragid].start = PAGESTACK->contents[i].start;
			PAGESTACK->contents[lastdefragid].length = PAGESTACK->contents[i].length;
		}
	};
}