/* Supertos Industries
 * MemoryRange implementation. MemoryRange describes severval continous MemoryPages (4K on x86).
 * This implementation uses Skip-List (SkipList.h) as well as mergin quirks for premature (before allocation) hugepage (2M on x86) forming.
 */

#include "SkipList.h"
#include "MemoryRange.h"
#include "utils/Architecture.h"
#include "utils/Random.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>


typedef struct MemoryRange MemoryRange;
struct MemoryRange {
    SkipList* SizeNode;
    SkipList* AddrNode;

    size_t Pages;
    uintptr_t Begin;
};


typedef struct PoolElement PoolElement;
struct PoolElement {
    PoolElement* Next;
};


/* Splits region in Pool elements of fixed size and updates pool top element for pool to contain new elements. */
void RegionToPool( PoolElement** top, void* begin, size_t length, size_t elementLength ) {
    if( length < elementLength || elementLength < sizeof(PoolElement) ) return;

    PoolElement* end = (PoolElement*)((uintptr_t)begin + length - elementLength);
    PoolElement* cur, *next = (PoolElement*)begin;
    do {
        cur = next;
        cur->Next = (PoolElement*)((uintptr_t*)cur + elementLength);
    } while( (next=cur->Next) <= end );

    cur->Next = *top;
    *top = (PoolElement*)begin;
}


/* Allocates element from pool. */
void* AllocateFromPool( PoolElement** top ) {
    if( !(*top) ) return NULL;

    void* out = *top;
    *top = (*top)->Next;
    return out;
}


/* Frees element to pool. */
void FreeToPool( PoolElement** top, void* element ) {
    if( !element ) return;

    ((PoolElement*)element)->Next = *top;
    *top = (PoolElement*)element;
}


/* Returns random node height based on geometrical law. */
size_t RandomMemoryRangeHeight( double p, size_t maxLevel ) {
	size_t level = 1;
	while( Random() < p && level < maxLevel ) ++level;
	
	return level;
}


/* Returns pointer to appropriate size skip list based on range's size and alignment. */
SkipList* MemoryRangeToSentinel( MemoryInfo* info, MemoryRange* range ) { 
    if( !range ) return NULL;

    if( range->Begin % HUGEPAGE_SIZE != 0 ) return &info->Unaligned2M;
    if( range->Pages >= PAGES_PER_HUGEPAGE ) return &info->Aligned2MBig;

    return &info->Aligned2M;
}


/* Sets Memory Range begin value and updates corresponding Skip-List weight.*/
void SetMemoryRangeBegin( MemoryInfo* info, MemoryRange* range, uintptr_t begin ) {
    range->Begin = begin;
    range->AddrNode->Weight = (size_t)begin;

    SkipListRemove( range->AddrNode );
    SkipListInsert( &info->Addr, range->AddrNode, RandomMemoryRangeHeight(MEMRANGE_ADDR_P, SKIPLIST_MAX_LEVELS) );
}


/* Sets Memory Range pages and updates corresponding Skip-List weight, possibly moving range between lists.*/
void SetMemoryRangePages( MemoryInfo* info, MemoryRange* range, size_t pages ) {
    range->Pages = pages;
    range->SizeNode->Weight = pages;
    
    SkipListRemove( range->SizeNode );
    SkipListInsert( MemoryRangeToSentinel(info, range), range->SizeNode, RandomMemoryRangeHeight(MEMRANGE_SIZE_P, SKIPLIST_MAX_LEVELS) );
}


/* Creates new MemoryRange and inserts it in corresponding Skip-Lists.
 * Function may not create new MemoryRange if memory is insufficient - in this case it uses provided memory region for pools.
 */
void NewMemoryRange( MemoryInfo* info, uintptr_t begin, size_t pages ) {
    MemoryRange* range = AllocateFromPool( &info->MemoryRangeFreeList );
    SkipList* sizeList = AllocateFromPool( &info->SkipListFreeList );
    SkipList* addrList = AllocateFromPool( &info->SkipListFreeList );

    if( !range && pages > 0 ) {
        RegionToPool( &info->MemoryRangeFreeList, (PoolElement*)(begin + (--pages) * PAGE_SIZE), PAGE_SIZE, sizeof(MemoryRange) );
        range = AllocateFromPool( &info->MemoryRangeFreeList );
    }

    if( (!addrList || !sizeList) && pages > 0 ) {
        RegionToPool( &info->SkipListFreeList,  (PoolElement*)(begin + (--pages) * PAGE_SIZE), PAGE_SIZE, sizeof(SkipList) );
        sizeList = sizeList ? sizeList : AllocateFromPool( &info->SkipListFreeList );
        addrList = addrList ? addrList : AllocateFromPool( &info->SkipListFreeList );
    }

    if( !addrList || !sizeList || !range || pages == 0 ) {
        FreeToPool( &info->SkipListFreeList, sizeList );
        FreeToPool( &info->SkipListFreeList, addrList );
        FreeToPool( &info->MemoryRangeFreeList, range );
        return;
    }

    SkipListInitNode(addrList);
    SkipListInitNode(sizeList);
    SetSkipListPayload(addrList, range);
    SetSkipListPayload(sizeList, range);
    
    *range = (MemoryRange){
        .AddrNode = addrList,
        .SizeNode = sizeList };
    
    SetMemoryRangeBegin( info, range, begin );
    SetMemoryRangePages( info, range, pages );

    MemoryRange* next = SkipListPayload(SkipListNext(range->AddrNode));
    TryMergeMemoryRangesWithPrev( info, next );
    TryMergeMemoryRangesWithPrev( info, range );
}


/* Removes MemoryRange and returns all it's data to corresponding pools. */
void MemoryRangeRemove( MemoryInfo* info, MemoryRange* range ) {
    SkipListRemove( range->AddrNode );
    SkipListRemove( range->SizeNode );

    FreeToPool( &info->SkipListFreeList, range->AddrNode );
    FreeToPool( &info->SkipListFreeList, range->SizeNode );

    FreeToPool( &info->MemoryRangeFreeList, range );
}


/* Tries merging MemoryRange with it's previous neighbor. 
 * Since this function is called on every insert, there can be no cascade merging.
 * This function may not merge to consecutive MemoryRanges to preserve alignment.
 */
void TryMergeMemoryRangesWithPrev( MemoryInfo* info, MemoryRange* range ) {
    if( !range ) return;
    MemoryRange* prev = SkipListPayload( SkipListPrev(range->AddrNode) );
    if( !prev ) return;
    if( prev->Begin + prev->Pages * PAGE_SIZE != range->Begin ) return;

    bool prevAligned = prev->Begin % HUGEPAGE_SIZE == 0;
    bool rangeAligned = range->Begin % HUGEPAGE_SIZE == 0;
    if( !prevAligned && rangeAligned ) return;
    
    SetMemoryRangePages( info, prev, prev->Pages + range->Pages );
    MemoryRangeRemove( info, range );
}


/* Inserts New MemoryRange in Skip-Lists. 
 * Function may not create new MemoryRange if memory is insufficient - in this case it uses provided memory region for pools.
 * Function may split the region in two based on it's alignment (or lack of it).
 */
void InsertNewMemoryRange( MemoryInfo* info, uintptr_t begin, size_t pages ) {
    if( pages == 0 ) return;
    size_t pagesToSplit = (HUGEPAGE_SIZE - begin % HUGEPAGE_SIZE) / PAGE_SIZE; // if begin % HUGEPAGE_SIZE == 0 then pagesToSplit == PAGES_PER_HUGEPAGE.

    if( pagesToSplit % PAGES_PER_HUGEPAGE != 0 && pagesToSplit < pages ) {
        NewMemoryRange( info, begin, pagesToSplit );
        NewMemoryRange( info, begin + pagesToSplit * PAGE_SIZE, pages - pagesToSplit );
    }else{
        NewMemoryRange( info, begin, pages );
    }
}


/* Allocates memory from provided Skip-List characterized by it's sentinel. 
 * Function may return less memory than requested if exact == false.
 */
bool AllocatePagesFromList( MemoryInfo* info, SkipList* sentinel, size_t* pages, void** begin, size_t pagesNeed, bool exact ) {
    SkipList* best = SkipListBest( sentinel, pagesNeed, exact );

    if( !best ) {
        *pages = 0;
        return false;
    }

    *pages = min(best->Weight, pagesNeed);
    uintptr_t rangeBegin = ((MemoryRange*)best->Payload)->Begin;
    uintptr_t allocBegin = rangeBegin + (best->Weight - (*pages)) * PAGE_SIZE;
    *begin = (void*)allocBegin;

    if( best->Weight == pagesNeed ) {
        MemoryRangeRemove( info, (MemoryRange*)best->Payload );
        return true;
    }
    
    SetMemoryRangePages( info, (MemoryRange*)best->Payload, ((MemoryRange*)best->Payload)->Pages - (*pages) );
    return true;
}


/* Allocates memory.
 * Function may return less memory than requested if exact == false.
 */
bool AllocatePages( MemoryInfo* info, size_t* pages, void** begin, size_t pagesNeed, bool exact ) {
    if( pagesNeed < PAGES_PER_HUGEPAGE && AllocatePagesFromList(info, &info->Unaligned2M, pages, begin, pagesNeed, exact) ) return true;
    if( pagesNeed < PAGES_PER_HUGEPAGE && AllocatePagesFromList(info, &info->Aligned2M, pages, begin, pagesNeed, exact) ) return true;

    return AllocatePagesFromList( info, &info->Aligned2MBig, pages, begin, pagesNeed, exact );
}