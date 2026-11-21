/* Supertos Industries (2012 - 2026) 
 * RBTree-based allocator with automatic coalescing.
 */
#pragma once

#define FIRST_FIT true
#define BEST_FIT false 

#include "Base.h"

#include "MemoryManager.h"

#include "Primitives/RBTree.h"
#include "Primitives/RWLock.h"

typedef struct {
    usize   Base;
	usize   End;
    RBIndex Index;
} RBEntry;

typedef struct {
    RBTree Free;
    Caches* Caches;
    RWLock Lock;
} RBAllocator;

RBAllocator* RBAllocatorNew( void* begin, Caches* caches );

void RBAllocatorSetCaches( RBAllocator* self, Caches* caches );

Allocator RBAlloc( RBAllocator* self, bool first );

void RBAllocatorEmpty( RBAllocator* self );