/* Supertos Industries (2012 - 2026) */

#include "VSpace.h"
#include "VRegion.h"

#include "RBAllocator.h"

static_assert( sizeof(VInclude) <= 64, "VIncludes is too big for internal SLAB." );


static inline PageWalker MyWalker( VSpace* self, bool shallMap ) {
	Allocator alloc = shallMap ? self->Tree->Allocator : AllocatorInvalid;
	return WalkerNew(self->Root, alloc);
}


static inline usize SelectAlign( usize pBase, usize vBase, usize length ) {
    usize aligns[] = {PAGE, HUGEPAGE, GIGAPAGE};
    usize align    = (pBase | vBase);

	usize i = 1;
    for( ; i < countof(aligns); i++ )
        if( align % aligns[i] != 0 || length < aligns[i] ) break;

    return aligns[i - 1];
}


bool VirtualMap( VSpace* self, Memory phys, Memory virt, char* mode, bool huge ) {
	if( !MemoryValid(phys) ) return false;
	if( !MemoryValid(virt) ) virt = phys;
	
    PageWalker walk = MyWalker(self, true);

    usize vBase = virt.Base, pBase = phys.Base;
	usize vEnd  = vBase + min(MemorySize(phys), MemorySize(virt));
	while( vBase < vEnd ) {
		usize align = huge ? SelectAlign(pBase, vBase, vEnd - vBase) : PAGE;
		PTE*  entry = WalkerEntry(&walk, vBase, align);

		if( entry == NULL ) return false;

		WalkerSetEntry(&walk, (void*)pBase, mode);
		if( align > PAGE ) WalkerSetHuge(&walk);

		vBase += align;
		pBase += align;
	}

    return true;
}


void VirtualUnmap( VSpace* self, Memory virt ) {
	if( !MemoryValid(virt) ) return;
    PageWalker walk = MyWalker(self, true);
    
	PTE* entry = WalkerEntry( &walk, virt.Base, PAGE );
	while( WalkerCurAddress(&walk) < virt.End ) {
		if( PTEAddr(entry) != NULL ) WalkerSetEntry( &walk, NULL, "" );
		entry = WalkerSkipNULLs(&walk, virt.End);
	}
}


static inline Memory ContiguousMemory( PageWalker* walk, usize vEnd, usize* pBase ) {
	PTE* cur = WalkerCurEntry(walk);
	cur = PTEAddr(cur) ? cur : WalkerSkipNULLs(walk, vEnd);
	
	usize vBase = WalkerCurAddress(walk), vCur = vBase;
	*pBase = (usize)PTEAddr(cur);

	while( PTEAddr(cur) != NULL && vCur < vEnd ) {
		if( (usize)PTEAddr(cur) - *pBase != vCur - vBase ) break;
		
		vCur = WalkerCurAddress(walk);
		cur = WalkerNext(walk);
	}
	
	return (Memory){ vBase, vCur };
}


bool VirtualBlit( VSpace* from, VSpace* to, Memory vSource, Memory vTarget, IMap* interface, char* mode ) {
	if( !MemoryValid(vTarget) || !MemoryValid(vSource) ) return false;

    PageWalker walk = MyWalker(from, false);
	WalkerEntry( &walk, vSource.Base, PAGE );

	usize pCur;
	Memory vChunk;
	while( MemoryValid(vChunk = ContiguousMemory(&walk, vSource.End, &pCur)) ) {
		usize chunkSize   = MemorySize(vChunk);
		usize chunkOffset = vChunk.Base - vSource.Base;
		
		Memory vTrg = MemoryNew(vSource.Base + chunkOffset, chunkSize);
		Memory pSrc = MemoryNew(pCur, chunkSize);
		
		Memory out = interface->OnMapAt( interface->Self, to, pSrc, vTrg, mode );
		if( !MemoryValid(out) ) return false;
	}

	return true;
}


void VirtualPurge( VSpace* to, Memory vTarget, IMap* interface ) {
    PageWalker walk = MyWalker(to, true);

	usize pCur;
	WalkerEntry( &walk, vTarget.Base, PAGE );
	Memory blit;
	while( MemoryValid(blit = ContiguousMemory(&walk, vTarget.End, &pCur)) )
		interface->OnUnmap( interface->Self, to, pCur, blit );
}


static VInclude* VIncludeNew( void* base, Memory to, VRegion* source, VSpace* space ) {
	if( base == NULL ) return NULL;
	VInclude* self = (VInclude*)base;
	*self = (VInclude){
		.Memory = to,
		.Region = source,
		.Space = space
	};
	
	return self;
}


VInclude* VRegionInclude( VSpace* self, VRegion* region, Memory to, char* mode ) {
	to = MemoryValid(to) ? to : FromVSpace( self->Allocator, MemorySize(region->Memory), PAGE );
	if( !MemoryValid(to) ) return false;
	
	VInclude* include = FromCaches(self->Tree->Caches, sizeof(VInclude));
	if( include == NULL ) return NULL;
	VIncludeNew(include, to, region, self);

	if( VRegionBlit(region, self, to, &include->Version, mode) ) {
		ToRBTree(self->Includes, include);
		return include;
	}

	ToCache(include);
	return NULL;
}


void VRegionDeclude( VSpace* self, VInclude* include ) {
	VRegionPurge( include->Region, self, include->Memory );
	
	FromRBTree(self->Includes, include);
	ToCache(include);
}


static void FromVTree( VTree* self, VSpace* el ) {
	ACQUIRE_AUTO(&self->Lock);
	FromRBTree(self->Spaces, el);
	el->Tree = NULL;
}


static void ToVTree( VTree* self, VSpace* el ) {
	ACQUIRE_AUTO(&self->Lock);
	ToRBTree(self->Spaces, el);
	el->Tree = self;
}


VSpace* VSpaceNew( VTree* tree ) {
	Memory  root = From( tree->Allocator, PTE_ROOT_SIZE, PTE_ROOT_ALIGN );
	VSpace* out  = FromCaches( tree->Caches, sizeof(VSpace) );
	if( !MemoryValid(root) || out == NULL ) goto cleanup;
	
	*out = (VSpace) {
		.Root 	   = PTNew((void*)root.Base),
		.Tree 	   = tree,
		.Allocator = SpaceVMANew( tree->Caches, out->Tree->VMA ),
		.Includes  = FromCaches(tree->Caches, sizeof(RBTree))
	};

	if( out->Allocator == NULL || out->Includes == NULL ) goto cleanup;
	RBTreeNew( &out->Includes, offsetof(VInclude, Index), offsetof(VInclude, Memory.Base) );
	ToVTree(tree, out);
	return out;

	[[gnu::cold]] cleanup:
		if( MemoryValid(root) ) To(tree->Allocator, root);
		if( out == NULL ) return NULL;

		ToCache(out);
		if( out->Allocator != NULL ) ToCache(out->Allocator);
		if( out->Includes != NULL ) ToCache(out->Includes);
		return NULL;
}


bool VSpaceDispose( VSpace* self ) {
	VInclude* inc;
	while( (inc = RBTreeRoot(self->Includes)) != NULL )
		VRegionDeclude(self, inc);

	SpaceVMADispose(self->Allocator);
	ToCache(self->Includes);

	VirtualUnmap(self, MemoryNew(0, self->Tree->MaxPAddr));

	To( self->Tree->Allocator, MemoryNew((usize)self->Root, PTE_ROOT_SIZE) );

	FromVTree(self->Tree, self);
	ToCache(self);
	return true;
}


VInclude* VSpaceInclude( VSpace* self, usize addr ) {
    return SearchRBTree(self->Includes, addr);
}

/* 
void VSpaceTransfer( VSpace* self, VTree* newOwner ) {
	FromVTree(self->Tree, self);

	SetSpaceVMACore(self->Allocator, core);
	
	VInclude* cur = NULL;
	while( (cur = RBTreeNext(self->Includes, cur)) != NULL )
		if( VRegionOwner(cur->Region) == self ) VRegionTransfer(cur->Region, newOwner->Caches);

	compiler_barrier;
	architecture_barrier;
	ToVTree(CoreVTree(core), self);
}
 */

void VSpaceEnter( VSpace* self, ContextID pcid, bool flush ) {
	if( self->Invalid ) flush = true;
		
	self->Invalid = false;
	LoadContext( (void*)self->Root, pcid, flush );
}


u64 VSpaceID( VSpace* self ) {
	return (usize)self->Root;
}


bool VIncludeUpgrade( VInclude* self, char* mode ) {
	if( self->Version == self->Region->Version ) return false;
	VRegionUpgrade( self->Region, self->Space, self->Memory, &self->Version, mode );
	return true;
}


VTree* VTreeNew( void* base, Frames* frames, Caches* caches, u64 maxPAddr, LocalVMA* lvma ) {
	VTree* self = (VTree*)base;
	*self = (VTree){0};
	*self = (VTree){ 
		.Allocator = FramesAlloc(frames),
		.Caches    = caches,
		.MaxPAddr  = maxPAddr,
		.Spaces    = FromCaches(caches, sizeof(VTree)),
		.VMA	   = lvma
	};
	if( &self->Spaces == NULL ) return NULL;
	RBTreeNew( self->Spaces, offsetof(VSpace, Index), offsetof(VSpace, Root) );
	return self;
}


VSpace* VTreeSpace( VTree* self, usize id ) {
	ACQUIRE_READER_AUTO(&self->Lock);
	return SearchRBTree(self->Spaces, id);
}


void VSpaceInvalidate( VSpace* self ) {
	u64 desc = (u64)ThisContext();
	if( desc == VSpaceID(self) )
		InvalidateContext(NULL, VSpaceID(self));
	else
		self->Invalid = true;
}