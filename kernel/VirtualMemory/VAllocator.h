/* Supertos Industries (2012 - 2026) 
 * VMA Hierarchy implementation.
 *
 * Shared Virtual Memory: normal vmem, no guarantees.
 * Exclusive Virtual Memory: vmem that is guaranteed to be free in every VirtualSpace. Used when consistency of pointers across different VirtualSpaces is needed.
 * 
 * 
 * GlobalVMA:
 *  Resides in Median NUMA (lowest avg. latency),
 *  Monotonically grows Bump's base and marks memory either shared or exclusive,
 *  Supports list of every shared memory ever created, used when new LocalVMA's created to catch up with others.
 *  Is the only entity to provide exclusive ranges.
 *  Shared list tends to slow down (or stop entirely) growing after a while since it accumulates huge amounts of virtual memory enough for any process.
 * 
 * LocalVMA:
 * Resides locally in every NUMA.
 * Caches GlobalVMA's shared memory list to provide newly created VirtualSpaces with usable shared virtual memory.
 * Local list growth slows down as well.
 * 
 * SpaceVMA:
 * Created for every Process Physical Memory Manager.
 * Tracks free memory memory to map to.
 * Supports free since it tracks actual memory usage.
 * If Source is NULL, relies on external code to replenish.
 */

#pragma once
 
#include "Base.h"

#include "PhysicalMemory/Cache.h"
#include "PhysicalMemory/Bump.h"
#include "RBAllocator.h"
#include "Primitives/List.h"
#include "Primitives/RBTree.h"

#include "MemoryManager.h"

#include "Primitives/RWLock.h"

typedef struct {
    usize     Base;
    usize     End;
    ListIndex Index;
} SVMAEntry;

typedef struct {
    Caches*    Caches;
    SVMAEntry* Last;
    List       Map;
    RWLock     Lock;
} VMASharedRanges;

typedef struct {
    SVMAEntry*       Cur;
    usize            Top;
    RWLock           Lock;
    VMASharedRanges* Source;
} SVMACursor;

struct GlobalVMA {
    VMASharedRanges Shared;
    Bump            Source;
    RBAllocator     Exclusive;
    Cache           ECache;
    Caches          Caches;
};

struct LocalVMA {
    VMASharedRanges Shared;
    SVMACursor      Source;
    GlobalVMA*      Global;
    Caches          Caches;
};

struct SpaceVMA{
    RBAllocator* Free;
    SVMACursor   Source;
    LocalVMA*    Local;
    RBTree*      Exclusive;
    Caches*      Caches;
};

Memory FromVSpace( SpaceVMA* self, usize size, usize align );

void ToVSpace( SpaceVMA* self, Memory entry );

Memory FromExclusiveVMA( SpaceVMA* self, usize size, usize align );

void ToExclusiveVMA( SpaceVMA* self, Memory entry );

SpaceVMA* SpaceVMANew( Caches* caches, LocalVMA* lvma );

void SpaceVMADispose( SpaceVMA* self );

/* void SetSpaceVMACore( SpaceVMA* self, Core* core ); */