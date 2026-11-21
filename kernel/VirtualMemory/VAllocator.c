/* Supertos Industries (2012 - 2026) */
#include "VAllocator.h"

typedef struct {
    RBIndex Index; 
    usize   Base;
    usize   End;
} ExclusiveNode;

static Memory __FromSharedRanges( SVMACursor* self, bool flush ) {
    ACQUIRE_AUTO(&self->Lock);
    VMASharedRanges* ranges = self->Source;

    ACQUIRE_READER_AUTO(&ranges->Lock);
    SVMAEntry *cur = self->Cur ?: ranges->Last, *prev = NULL;
    usize curAddr = self->Top;
	
    while( cur && curAddr >= cur->End ) {
        cur = ListPrev(&ranges->Map, cur); // List quirk: insertions are front. Prev - higher addr.
		if( flush && prev && (usize)prev / CACHELINE != (usize)cur / CACHELINE ) FlushCachelines(MemoryNew(prev, sizeof(SVMAEntry)));
		prev = cur;
	}

    if( cur == NULL ) return MemoryInvalid;
    self->Cur = cur;
    self->Top = cur->End;

	if( flush ) FlushCachelines(MemoryNew(cur, sizeof(SVMAEntry)));
    return (Memory){ max(cur->Base, curAddr), cur->End };
}


static Memory FromSharedRangesFlush( SVMACursor* self, unused usize size, unused usize align ) { 
	return __FromSharedRanges( self, true );
}


static Memory FromSharedRanges( SVMACursor* self, unused usize size, unused usize align ) { 
	return __FromSharedRanges( self, false );
}


static void ToSharedRanges( SVMACursor* self, Memory entry ) {
    ACQUIRE_AUTO(&self->Lock);
    VMASharedRanges* ranges = self->Source;

    ACQUIRE_AUTO(&ranges->Lock);
    SVMAEntry* top = ListFirst(&ranges->Map);
    if( top && top->End == entry.Base ) { 
        top->End = entry.End; 
        return; 
    }

    top = FromCaches(ranges->Caches, sizeof(SVMAEntry));
    *top = (SVMAEntry){ .Base = entry.Base, .End = entry.End };
    ToList( &ranges->Map, top );

    if( !ranges->Last ) ranges->Last = top;
}


Memory FromVSpace( SpaceVMA* self, usize size, usize align ) {
    Allocator mainAlloc = RBAlloc(self->Free, FIRST_FIT);

    Memory out = From( mainAlloc, size, align );
    if( MemorySuits(out, size, align) ) return out;
    
    LocalVMA* local = self->Local;
    GlobalVMA* global = local->Global;

    Allocator Flow[] = {
        mainAlloc,
        AllocatorNew( FromSharedRanges, ToSharedRanges, &self->Source ),
        AllocatorNew( FromSharedRangesFlush, ToSharedRanges, &local->Source ),
        AllocatorNew( FromBump, NULL, &global->Source )
    };
    
    usize i = 1;
    for( ; !MemorySuits(out, size, align); i++ )
        out = Replenish( Flow[i], Flow[i - 1], size, align );

    for( ; i > 2; i-- )
        Replenish( Flow[i - 2], Flow[i - 3], size, align );
    
    return From( mainAlloc, size, align );
}


void ToVSpace( SpaceVMA* self, Memory entry ) {
    Allocator alloc = RBAlloc(self->Free, FIRST_FIT);
    To(alloc, entry);
}


Memory FromExclusiveVMA( SpaceVMA* self, usize size, usize align ) {
    GlobalVMA* vma = self->Local->Global;
    
    ExclusiveNode* node = FromCaches(self->Caches, sizeof(ExclusiveNode));
    if( node == NULL ) return MemoryInvalid;
    
    Allocator alloc = RBAlloc(&vma->Exclusive, BEST_FIT);
    
    Memory out = From( alloc, size, align );
    if( MemorySuits(out, size, align) ) return out;
        
    Allocator source = AllocatorNew( FromBump, NULL, &vma->Source );
    Replenish(source, alloc, size, align);

    out = From(alloc, size, align);

    *node = (ExclusiveNode){ .Base = out.Base, .End = out.End };
    ToRBTree(self->Exclusive, node);

    return out;
}


void ToExclusiveVMA( SpaceVMA* self, Memory entry ) {
    ExclusiveNode* node = SearchRBTree(self->Exclusive, entry.Base);
    if( node == NULL ) return;
    ToCache( FromRBTree(self->Exclusive, node) );

    GlobalVMA* vma = self->Local->Global;
    To( RBAlloc(&vma->Exclusive, BEST_FIT), entry );
}


GlobalVMA* GlobalVMANew( void* begin, Caches* caches, usize maxPhysAddr ) {
	GlobalVMA* self = (GlobalVMA*)begin;
	*self = (GlobalVMA) {
		.Shared = (VMASharedRanges){ .Caches = caches },
		.Source = BumpNew(maxPhysAddr, MASK(usize, 0, VIRT_BITS) - maxPhysAddr, "GLOBAL_VMA")
	};

	RBAllocatorNew( &self->Exclusive, caches );

	ListNew( &self->Shared.Map, offsetof(SVMAEntry, Index) );
	return self;
}


LocalVMA* LocalVMANew( void* begin, Caches* caches, GlobalVMA* gvma ) {
	LocalVMA* self = begin; 
	*self = (LocalVMA){
		.Shared = (VMASharedRanges){ .Caches = caches },
		.Source = (SVMACursor){ .Source = &gvma->Shared },
        .Global = gvma
	};

	ListNew( &self->Shared.Map, offsetof(SVMAEntry, Index) );
	return self;
}

SpaceVMA* SpaceVMANew( Caches* caches, LocalVMA* lvma ) {
    SpaceVMA*    self  = FromCaches(caches, sizeof(SpaceVMA));
    RBAllocator* alloc = FromCaches(caches, sizeof(RBAllocator));
    RBTree*      tree  = FromCaches(caches, sizeof(RBTree));

    *self = (SpaceVMA){
        .Local     = lvma,
        .Source    = (SVMACursor){ .Source = &lvma->Shared },
        .Free      = RBAllocatorNew(alloc, caches), 
        .Exclusive = RBTreeNew(tree, offsetof(ExclusiveNode, Index), offsetof(ExclusiveNode, Base)),
        .Caches    = caches
    };
    return self;
}

void SpaceVMADispose( SpaceVMA* self ) {
    RBAllocatorEmpty(self->Free);

    ExclusiveNode* first = NULL;
    while( (first = RBTreeFirst(self->Exclusive)) != NULL )
        ToExclusiveVMA(self, (Memory){first->Base, first->End});

    ToCache(self->Free);
    ToCache(self);
}



/* 

void SetSpaceVMACore( SpaceVMA* self, Core* core ) {
    RBAllocatorSetCache( self->Free, CoreCache(core, sizeof(RBEntry)) );
    if( CoreVMA(core) != self->Local ) return;
    
    self->Local = CoreVMA(core);
    self->Source = (SVMACursor){ .Source = &CoreVMA(core)->Shared };
}
    */