/* Supertos Industries (2012 - 2026) 
 * VirtualSpace is modified only by owner CPU.
 * 
 * VirtualRegions system utilises epoch-based reclamation required by kernel structures mapped into userspace.
 * VirtualFamily describes region in VirtualSpace, that shall be blit onto other VirtualSpaces.
 * VirtualRegion describes single version, and can only be modified/blit onto when is the newest.
 * Outdated VirtualRegions (those that have predecessor (front insertion)) are alive as long as single VirtualSpace has it included.
 * Upon last reference lost (and virtual mappings purged + physics freed if THEIR reference count is zero), VirtualRegion is destroyed.
 * 
 * VirtualSpace can include only newest VirtualRegion.
 * Newest VirtualRegion has at least one reference (that is, owner VirtualSpace), and will never be removed unless VSpaceDispose is called.
 */
#pragma once

#include <assert.h>

#include "Base.h"

#include "Primitives/RWLock.h"

#include "RBAllocator.h"
#include "VSpace.h"
#include "MemoryManager.h"

struct VRegion {
    RBAllocator*  Allocator;
    VSpace*       Space;
    IMap*         Interface;
    Memory        Memory;
    RWLock        Lock;
    RegionVersion Version;
    u32           ExternalReferences;
};

static_assert( sizeof(VRegion) <= 64, "VRegion is too large for internal meta SLAB!" );

bool VRegionBlit( VRegion* self, VSpace* space, Memory trg, RegionVersion* ver, char* mode );

void VRegionPurge( VRegion* self, VSpace* space, Memory trg );

VRegion* VRegionNew( VSpace* owner, Caches* caches, Memory virt, IMap* interface );

bool VRegionDispose( VRegion* self );

Memory FromVRegion( VRegion* self, usize size, usize align );

Memory VRegionMap( VRegion* self, usize size, usize align, char* mode );

Memory VRegionMapAt( VRegion* self, Memory phys, Memory virt, char* mode );

void VRegionUnmap( VRegion* self, Memory virt );

void LockVRegion( VRegion* self );

void ReleaseVRegion( VRegion* self );

VSpace* VRegionOwner( VRegion* self );

bool VRegionUpgrade( VRegion* self, VSpace* space, Memory trg, RegionVersion* ver, char* mode );

void VRegionTransfer( VRegion* self, Caches* caches );