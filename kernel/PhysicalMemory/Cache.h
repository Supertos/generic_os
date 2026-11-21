/* Supertos Industries (2012 - 2026) 
 * Collection of SLABs objects with automatic lifetime control.
 */
#pragma once

#include "Base.h"

#include "Primitives/RWLock.h"
#include "Primitives/List.h"
#include "Primitives/Bitmap.h"

#include "MemoryManager.h"

#define CACHE_SIZES    3	// 16, 32, 64
#define CACHE_MIN_SIZE 16
#define CACHE_MAX_SIZE (CACHE_MIN_SIZE << CACHE_SIZES)

struct Cache {
	RWLock InUse;
	List Fragments;
	u16 BinSize;
	
	Allocator Source;
};

struct Caches {
	Cache Sizes[CACHE_SIZES];
};