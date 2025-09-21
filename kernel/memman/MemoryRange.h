/* Supertos Industries
 * MemoryRange implementation. MemoryRange describes severval continous MemoryPages (4K on x86).
 * This implementation uses Skip-List (SkipList.h) as well as mergin quirks for premature (before allocation) hugepage (2M on x86) forming.
 */
#pragma once
#include "SkipList.h"

#include <stdbool.h>
#include <stdint.h>

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#define MEMRANGE_ADDR_P 0.5
#define MEMRANGE_SIZE_P 0.25

typedef struct MemoryRange MemoryRange;
struct MemoryRange;


typedef struct PoolElement PoolElement;
struct PoolElement;


typedef struct MemoryInfo MemoryInfo;
struct MemoryInfo {
    PoolElement* MemoryRangeFreeList;
    PoolElement* SkipListFreeList;

    SkipList Unaligned2M;
    SkipList Aligned2MBig;
    SkipList Aligned2M;

    SkipList Addr;
};


/* Splits region in Pool elements of fixed size and updates pool top element for pool to contain new elements. */
void RegionToPool( PoolElement** top, void* begin, size_t length, size_t elementLength );


/* Inserts New MemoryRange in Skip-Lists. 
 * Function may not create new MemoryRange if memory is insufficient - in this case it uses provided memory region for pools.
 * Function may split the region in two based on it's alignment (or lack of it).
 */
void InsertNewMemoryRange( MemoryInfo* info, uintptr_t begin, size_t pages );


/* Allocates memory.
 * Function may return less memory than requested if exact == false.
 */
bool AllocatePages( MemoryInfo* info, size_t* pages, void** begin, size_t pagesNeed, bool exact );