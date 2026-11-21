/* Supertos Industries (2012 - 2026) */
#include "VRegion.h"


static inline bool VRegionOverwrite( VRegion* self, VSpace* space, Memory trg, RegionVersion* ver, char* mode ) {
	ACQUIRE_READER_AUTO(&self->Lock);
	VirtualPurge( space, trg, self->Interface );

	bool success = VirtualBlit( self->Space, space, self->Memory, trg, self->Interface, mode );
	if( !success ) 
		VirtualPurge( space, trg, self->Interface );
	else
		*ver = self->Version;
	
	VSpaceInvalidate(space);

	return success;
}


bool VRegionBlit( VRegion* self, VSpace* space, Memory trg, RegionVersion* ver, char* mode ) {
	if( !VRegionOverwrite( self, space, trg, ver, mode ) ) return false;
	FetchAdd32(&self->ExternalReferences, 1);
	return true;
}


void VRegionPurge( VRegion* self, VSpace* space, Memory trg ) {
	ACQUIRE_READER_AUTO(&self->Lock);
	VirtualPurge( space, trg, self->Interface );
	
	FetchSub32(&self->ExternalReferences, 1);
	VSpaceInvalidate(space);
}


bool VRegionUpgrade( VRegion* self, VSpace* space, Memory trg, RegionVersion* ver, char* mode ) {
	return VRegionOverwrite( self, space, trg, ver, mode );
}


VRegion* VRegionNew( VSpace* owner, Caches* caches, Memory virt, IMap* interface ) {
	VRegion* self = FromCaches(caches, sizeof(VRegion));
	if( self == NULL ) return NULL;

	RBAllocator* alloc = FromCaches(caches, sizeof(RBAllocator));
	if( alloc == NULL ) {
		ToCache(self);
		return NULL;
	}

	*self = (VRegion) {
		.Allocator = RBAllocatorNew(alloc, caches),
		.Interface = interface,
		.Space = owner,
		.Memory = virt,
		.ExternalReferences = 0
	};

	To( RBAlloc(self->Allocator, FIRST_FIT), virt );
	VRegionInclude( owner, self, virt, "" );
	return self;
}


bool VRegionDispose( VRegion* self ) {
	AcquireWriter(&self->Lock);
	u32 refs = self->ExternalReferences;
	ReleaseWriter(&self->Lock);
	if( refs > 1 ) return false;

	VRegionDeclude( self->Space, VSpaceInclude(self->Space, self->Memory.Base) );
	
	ToCache(self->Allocator);
	ToCache(self);
	return true;
}


Memory FromVRegion( VRegion* self, usize size, usize align ) {
	Allocator alloc = RBAlloc(self->Allocator, FIRST_FIT);
	return alloc.From( alloc.Self, size, align );
}


Memory VRegionMap( VRegion* self, usize size, usize align, char* mode ) { // Under LockVRegion
	Memory trg = FromVRegion(self, size, align);
	if( !MemoryValid(trg) ) return MemoryInvalid;

	return self->Interface->OnMap( self->Interface->Self, self->Space, trg, mode );
}


Memory VRegionMapAt( VRegion* self, Memory phys, Memory virt, char* mode ) {  // Under LockVRegion
	if( !MemoryValid(virt) ) virt = FromVRegion(self, MemorySize(phys), PAGE);
	if( !MemoryValid(virt) ) return MemoryInvalid;

	return self->Interface->OnMapAt( self->Interface->Self, self->Space, phys, virt, mode );
}


void VRegionUnmap( VRegion* self, Memory virt ) {  // Under LockVRegion
	VirtualPurge(self->Space, virt, self->Interface);
}


void LockVRegion( VRegion* self ) {
	AcquireWriter(&self->Lock);
}


void ReleaseVRegion( VRegion* self ) {
	FetchAdd16(&self->Version, 1);
	ReleaseWriter(&self->Lock);
}


void VRegionTransfer( VRegion* self, Caches* caches ) {
	RBAllocatorSetCaches( self->Allocator, caches );
}


VSpace* VRegionOwner( VRegion* self ) {
	return self->Space;
}