/* Supertos Industries (2012 - 2026) 
 * VSpace - Abstraction layer over MMU mechanism commonly found on x86 and ARM architectures.
 * The main difference from typical VMM subsystems is responsibility pyramid:
 * User -> VMM (Locate region and its MapInterface) -> MapInterface (Execute policy over VMM) -> VMM (Mapping, unmapping, vm allocation)
 */
#pragma once

#include "Base.h"
#include "MemoryManager.h"

#include "VAllocator.h"

#define ALLOW_HUGE 	  true
#define DISALLOW_HUGE false

struct VTree {
	RWLock 	   Lock;
	RBTree*    Spaces;
	Allocator  Allocator;
	Caches*	   Caches;
	u64		   MaxPAddr;
	LocalVMA*  VMA;
};


struct VSpace{
	void* 	  Root;
	VTree* 	  Tree;
    SpaceVMA* Allocator;
	RBTree*   Includes;
	RBIndex   Index;
	bool	  Invalid;
};


struct VInclude {
	RBIndex 	  Index;
    Memory 		  Memory;
    RegionVersion Version;
    VRegion* 	  Region;
	VSpace* 	  Space;
};


static_assert( sizeof(VSpace) <= 64, "VSpace is too big for internal SLAB." );
static_assert( sizeof(SpaceVMA) <= 64, "SpaceVMA is too big for internal SLAB." );
static_assert( sizeof(VTree) <= 64, "VTree is too big for internal SLAB." );
static_assert( sizeof(VInclude) <= 64, "VInclude is too big for internal SLAB." );
static_assert( sizeof(RBAllocator) <= 64, "RBAllocator is too big for internal SLAB." );


bool VirtualBlit( VSpace* from, VSpace* to, Memory vSource, Memory vTarget, IMap* interface, char* mode );

void VirtualPurge( VSpace* to, Memory vTarget, IMap* interface );

VInclude* VRegionInclude( VSpace* self, VRegion* region, Memory to, char* mode );

void VRegionDeclude( VSpace* self, VInclude* include );

VSpace* VSpaceNew( VTree* tree );

bool VSpaceDispose( VSpace* self );

VInclude* VSpaceInclude( VSpace* self, usize addr );

// void VSpaceTransfer( VSpace* self, Core* core );

void VSpaceEnter( VSpace* self, ContextID pcid, bool flush );

u64 VSpaceID( VSpace* self );

bool VIncludeUpgrade( VInclude* self, char* mode );

VTree* VTreeNew( void* base, Frames* frames, Caches* caches, u64 maxPAddr, LocalVMA* lvma );

VSpace* VTreeSpace( VTree* self, usize id );

void VSpaceInvalidate( VSpace* self );

SpaceVMA* VSpaceVMA( VSpace* self );