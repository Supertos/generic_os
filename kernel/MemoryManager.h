/* Supertos Industries (2012 - 2026) */
#pragma once

#include "Base.h"

typedef struct GlobalVMA GlobalVMA;
typedef struct LocalVMA  LocalVMA;
typedef struct SpaceVMA  SpaceVMA;

typedef struct VSpace 	 VSpace;
typedef struct VRegion 	 VRegion;
typedef struct VInclude  VInclude;
typedef u16 			 RegionVersion;

typedef struct VTree 	 VTree;

typedef struct Frames 	 Frames;
typedef struct Cache 	 Cache;
typedef struct Caches 	 Caches;

typedef struct {
	// Map specific physics at specific virtual address. IMap is expected to map phys at virt.
	Memory (*OnMapAt)( void* ppm, void* space, Memory phys, Memory virt, char* mode );

	// Map any physics at provided address. IMap is expected to return mapped vBegin.
	// Return any valid Memory even if memory is yet to be mapped. 
	Memory (*OnMap)( void* ppm, void* space, Memory virt, char* mode );

	// Unmap vBegin (and decrease reference count if supported)
	// API guarantees that [vBegin, vBegin + length] corresponds to [pBegin, pBegin + length]
	void (*OnUnmap)( void* ppm, void* space, usize pBegin, Memory virt );
	
	// Return false on critical failure, return true on success.
	// May be used as SynchroPoint to map reserved physics.
	bool (*PageFault)( void* ppm, void* region, usize pAddr, void* vAddr );
	void* Self;
} IMap;

void ToFrames( void* obj, Memory in );

void ToFramesNoLock( void* obj, Memory in );

Memory FromFrames( void* obj, u64 request, usize align );

Frames* FramesNew( void* base );

Frames* FramesMove( Frames* old, Frames* new );

Allocator FramesAlloc( Frames* self );

bool VirtualMap( VSpace* self, Memory phys, Memory virt, char* mode, bool huge );

void VirtualUnmap( VSpace* self, Memory virt );

GlobalVMA* GlobalVMANew( void* begin, Caches* caches, usize maxPhysAddr );

void* FromCaches( Caches* self, usize size );

void* FromCache( Cache* self );

Cache* CacheNew( void* base, Allocator source, usize binSize );

u64 CacheSize();

u64 CachesSize();

Caches* CachesNew( void* base, Allocator source );

void ToCache( void* addr );

LocalVMA* LocalVMANew( void* begin, Caches* caches, GlobalVMA* gvma );

Memory FromMemory( usize size );

void ToMemory( Memory in );