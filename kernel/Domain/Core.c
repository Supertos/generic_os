/* Supertos Industries (2012 - 2026) */
#include "Core.h"
#include "VirtualMemory/VSpace.h"


Core* ThisCore() {
	u64 regContents = ThisStorage();
	return (Core*)regContents;
}


Core* CoreNew( void* base, usize physMemory, Domain* domain, RootTable* root, CoreID id ) {
	Frames* frames = &domain->Memory;
	Core* self = base;
	*self = (Core){ 
		.SMPID = id,
		.Self  = base,
		.Owner = domain
	};

	CachesNew(&self->Caches, FramesAlloc(frames));
	if( VTreeNew(&self->Spaces, frames, &self->Caches, physMemory, &domain->VirtualRanges) == NULL ) 
		goto fail;
	
	VSpace* kernelSpace = VSpaceNew(&self->Spaces);
	
	if( kernelSpace == NULL ) goto fail;
	
	MemoryEntry* map = root->MemoryMap;
	for( usize i = 0; i < root->MemMapCount; i++ ) {
		MemoryEntry entr = map[i];
		VirtualMap(kernelSpace, entr.Range, MemoryInvalid, "RWK", ALLOW_HUGE);
	}

	VirtualMap(kernelSpace, root->ELFLoadedRange, MemoryInvalid, "RXK", DISALLOW_HUGE);

	VSpaceInvalidate(kernelSpace);
	self->KernelSpace = kernelSpace;

	return self;

	[[gnu::cold]] fail:
		KernelCrashUndBurn("Can't create core.");
		return NULL;
}


DomainID CoreSMPID( Core* self ) {
	return self->SMPID;
}


VTree* CoreVTree( Core* self ) {
	return &self->Spaces;
}


void ClaimCore( Core* self ) {
	self->Self = self;
	SetStorage(false, (void*)&self->Self);
}

Memory FromMemory( usize size ) {
	if( size <= CACHE_MAX_SIZE ) {
		void* obj = FromCaches( &ThisCore()->Caches, size );
		return MemoryNew((u64)obj, size);
	}

	if( size < PAGE ) KernelCrashUndBurn("Invalid allocation.");
	return FromAnyDomain( ThisCore()->Owner, size, 0 );
}

void ToMemory( Memory in ) {
	if( MemorySize(in) <= CACHE_MAX_SIZE ) {
		ToCache((void*)in.Base);
	}
	ToDomains( ThisCore()->Owner, in );
}