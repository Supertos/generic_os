/* Supertos Industries (2012 - 2026) */
#include "Frames.h"
#include "AddrNode.h"


#define BIT_HAS_HUGE    (1ULL << (bitsof(U64Frame) - 1))

#define SHIFT_HUGE      0
#define SHIFT_PAGE      PHYS_BITS

#define MASK_ADDR(huge) MASK( U64Frame, (huge ? 20 : 11), (SHIFT_PAGE - (huge ? 20 : 11)) )
#define MASK_HUGE       MASK( U64Frame, 0, 20 )
#define MASK_PAGE       MASK( U64Frame, SHIFT_PAGE, bitsof(U64Frame) - SHIFT_PAGE - 1 )

#define BUCKET_BY_ID(major, minor) (((major * (major + 1)) >> 1) + minor)


static inline Memory Unpack( U64Frame self ) {
    bool  hasHuge = self & BIT_HAS_HUGE;
    usize huges   = (hasHuge ? ((self & MASK_HUGE) << SHIFT_HUGE) : 0);
    usize pages   = (self & MASK_PAGE) >> SHIFT_PAGE;
    usize base    = (self & MASK_ADDR(hasHuge));

    return (Memory){ base, base + huges * HUGEPAGE + pages * PAGE };
}


static inline U64Frame Pack( Memory self ) {
    u64  huges   = (MemorySize(self) >> HUGE_BITS);
    u64  pages   = (MemorySize(self) & MASK(u64, 0, HUGE_BITS)) >> PAGE_BITS;
    bool hasHuge = huges > 0;

    U64Frame out = (self.Base & MASK_ADDR(hasHuge)) |
                   (huges << SHIFT_HUGE)            |
                   (pages << SHIFT_PAGE)            |
                   (hasHuge ? BIT_HAS_HUGE : 0)     ;
    return out;
}


static inline u64 MajorBySize( u64 size ) {
    size >>= PAGE_BITS;
    if( size >= HUGE_PAGES ) return SIZE_HUGE_LIST;

    u64 mask = 0b0101010101010101010101010101010101010101010101010101010101010101;
    size = ((size & ~mask) >> 1) | (size & mask);

    u64 major = (bitsof(size) - LZCNT64(size) - 1) >> 1;
    return major;
}


static inline u64 ViewListID( Memory mem ) {
    u64 size = MemorySize(mem) / PAGE;
    if( size >= HUGE_PAGES ) return SIZE_HUGE_LIST;

    u64 major = MajorBySize(size * PAGE);
    u64 minor = (TZCNT64(size) >> 1);

    return ((major * (major + 1)) >> 1) + minor;
}


static inline Frame* Meta( Memory mem ) {
    return (Frame*)mem.Base;
}


static inline SizeNode* Node( Memory mem ) {
    Frame* nodeEnd = Meta(mem) + sizeof(Frame*);
    return (SizeNode*)AlignUp(nodeEnd, SIZE_NODE_SIZE);
}


static inline SizeNode* CacheNode( may_alias U64Stack* stack ) {
    return (SizeNode*)((u64)stack - offsetof(SizeNode, Cache));
}


static inline u64 MaxFrameEnd( usize base, usize end ) {
    if( !Aligned(base, HUGEPAGE) ) return min( end, AlignUp(base, HUGEPAGE) );
    return min( end, max(base + HUGEPAGE, AlignDown(end, HUGEPAGE)) );
}


static inline U64Frame* ToSizeList( Frames* self, usize id, Memory mem ) {
    List*     list  = &self->Lists[id];
    SizeNode* cur   = ListFirst(list);
    if( cur == NULL || cur->Cache.Count == SIZE_NODE_ELMS ) {
        cur = ToList(list, Node(mem));
        U64CacheNew( &Node(mem)->Cache, SIZE_NODE_ELMS );
    }

    Meta(mem)->EntryPtr = ToU64Cache( &cur->Cache, Pack(mem) );
    return Meta(mem)->EntryPtr;
}


static inline Memory FromSizeList( Frames* self, usize id, U64Frame* item ) {
    List*     list  = &self->Lists[id];
    U64Stack* cache = ItemU64Cache(item);
    Memory    out   = Unpack(FromU64Cache(item));
   
    if( Node(out) == CacheNode(cache) ) {
        FromList(list, Node(out));
        while( cache->Count > 0 )
            ToSizeList( self, id, Unpack(FromU64Cache(cache->Items)) );
    }
    
    return out;
}


static inline U64Frame* UpdateSizeList( Frames* self, U64Frame* oldPtr, Memory new ) {
    Memory old    = Unpack(*oldPtr);
    u64    oldID  = ViewListID(old);
    u64    newID  = ViewListID(new);

    bool   rebase = (new.Base != old.Base);
    bool   move   = (oldID != newID);
    bool   remove = !MemoryValid(new);

    if( !remove && !rebase && !move ) {
        *oldPtr = Pack(new);
        return oldPtr;
    }
    
    FromSizeList(self, oldID, oldPtr);
    if( remove ) {
        FromAddrTree(self, Meta(old));
        return NULL;
    }

    if( rebase ) {
        *Meta(new) = *Meta(old);
        UpdateAddrReference( Meta(new)->OwnerPtr, (u64)Meta(new) );
    }

    return ToSizeList(self, newID, new);
}


static inline Memory ShrinkTopNode( Frames* self, usize major, usize minor, usize request ) {
    usize     id   = BUCKET_BY_ID(major, minor);
    List*     list = &self->Lists[id];
    SizeNode* cur = ListFirst(list);

    if( cur == NULL ) return MemoryInvalid;
    U64Frame* frame    = U64CacheTop(&cur->Cache);
    Memory    mem      = Unpack(*frame);

    if( request > MemorySize(mem) ) return MemoryInvalid;
    usize     maxDrain = mem.End - max(mem.Base, AlignDown(mem.End - 1, HUGEPAGE));
    usize     drain    = min(maxDrain, request);
    
    Memory    out = MemoryDrain(&mem, drain);
    self->Bytes  -= drain;

    UpdateSizeList(self, frame, mem);
    return out;
}


static inline Memory AppendToFrame( Frames* self, u64** addrEntry, Memory new ) {
    if( *addrEntry == NULL ) return new;
    
    Frame*    frame  = (Frame*)(**addrEntry);
    U64Frame* oldPtr = frame->EntryPtr; 
    Memory old = Unpack(*oldPtr);

    if( old.Base != new.End && old.End != new.Base ) return new;

    usize base  = min(old.Base, new.Base);
    usize end   = max(old.End, new.End);
    usize rEnd  = MaxFrameEnd(base, end);
    usize iBase = old.Base;

    Memory left  = (Memory){base, rEnd};
    Memory right = (Memory){rEnd, end};

    bool frameIsLeft = rEnd > iBase;
    
    UpdateSizeList(self, oldPtr, frameIsLeft ? left : right);
    *addrEntry = frame->OwnerPtr;

    return frameIsLeft ? right : left;
}

void ToFramesNoLock( void* obj, Memory in ) {
    Frames* self = (Frames*)obj;
    U64Result res = SearchAddrTree(self, in.Base);
    if( res.Trg != NULL ) KernelCrashUndBurn("Double free.");

    self->Bytes += MemorySize(in);

    in = AppendToFrame(self, &res.Prev, in);
    in = AppendToFrame(self, &res.Next, in);
    if( res.Prev != NULL && res.Next != NULL ) {
        Memory prev = Unpack(*res.Prev);
        Memory next = Unpack(*res.Next);

        if( prev.End == next.Base && MaxFrameEnd(prev.Base, next.End) == next.End ) {
            UpdateSizeList(self, res.Next, MemoryInvalid);
            AppendToFrame(self, &res.Prev, next);
        }
    }
    
    if( !MemoryValid(in) ) return;
    *Meta(in) = (Frame){0};

    ToAddrTree(self, Meta(in), &in);    
    if( !MemoryValid(in) ) return;

    ToSizeList(self, ViewListID(in), in);
}

void ToFrames( void* obj, Memory in ) {
    if( !MemoryValid(in) ) return;

    Memory huge = (Memory){ AlignUp(in.Base, HUGEPAGE), AlignDown(in.End, HUGEPAGE) };

    bool crossesAlign = AlignDown(in.Base, HUGEPAGE) != AlignDown(in.End, HUGEPAGE) &&
                        AlignUp(in.Base, HUGEPAGE) != AlignUp(in.End, HUGEPAGE) && 
                        !MemoryEqual(huge, in) && (in.End > huge.End || in.Base < huge.Base);
    if( crossesAlign ) {
        ToFrames( obj, (Memory){ in.Base, huge.Base } );
        ToFrames( obj, (Memory){ huge.End, in.End } );
        in = huge;
        if( !MemoryValid(in) ) return;
    }

    Frames* self = (Frames*)obj;
    ACQUIRE_AUTO(&self->Lock);
    ToFramesNoLock(obj, in);
}


// i = 0 means 0, any i > 0 means baseMajor + i - 1
static usize majors[SIZE_LISTS] = {
    1, // Best fit
    2, 3, 4, 5, // Bigger major, same minor
    3, 4, 4, 5, 5, 5, // Bigger major, bigger minor
    2, 3, 4, 5 // Bigger major, max minor
};

static usize minors[SIZE_LISTS] = {
    0,
    1, 1, 1, 1,
    2, 2, 3, 2, 3, 4,
    2, 3, 4, 5
};


Memory FromFrames( void* obj, u64 request, unused usize align ) {
    if( request == 0 || request > HUGEPAGE ) return MemoryInvalid;
    request = AlignUp(request, PAGE);

    Frames* self = (Frames*)obj;
    
    {
        ACQUIRE_AUTO(&self->Lock);

        if( request == HUGEPAGE ) 
            return ShrinkTopNode( self, 0, SIZE_HUGE_LIST, request );
        
        usize baseMajor = MajorBySize(request);
        for( usize i = 0; i < SIZE_CONV_LISTS; i++ ) {
            usize major = (majors[i] == 0) ? 0 : baseMajor + majors[i] - 1;
            usize minor = (minors[i] == 0) ? 0 : baseMajor + minors[i] - 1;

            if( major >= SIZE_POWERS || minor >= SIZE_POWERS || minor > major ) continue;
            Memory out = ShrinkTopNode( self, major, minor, request );
            if( !MemoryValid(out) ) continue;
            self->Bytes -= request;
            return out;
        }
    }

    AcquireWriter(&self->Lock);
    Memory split = ShrinkTopNode( self, 0, SIZE_HUGE_LIST, HUGEPAGE );
    if( !MemoryValid(split) ) {
        ReleaseWriter(&self->Lock);
        return MemoryInvalid;
    }

    Memory out = MemoryDrain(&split, request);
    self->Bytes -= request;
    ReleaseWriter(&self->Lock);

    ToFrames(self, split);

    return out;
}


Frames* FramesNew( void* base ) {
    Frames* out = (Frames*)base;
    *out = (Frames){0};

    RBTreeNew( &out->AddrTree, offsetof(AddrNode, Index), offsetof(AddrNode, Key) );
    for( usize i = 0; i < countof(out->Lists); i++ )
        ListNew( &out->Lists[i], offsetof(SizeNode, Index) );

    USLABNew(&out->Slab, out);
    return out;
}


Frames* FramesMove( Frames* old, Frames* new ) {
    for( usize i = 0; i < SIZE_LISTS; i++ ) {
        while( ListFirst(&old->Lists[i]) != NULL ) {
            SizeNode* first    = ListFirst(&old->Lists[i]);
            Memory    entry    = FromSizeList(old, i, U64CacheTop(&first->Cache));

            FromAddrTree( old, Meta(entry) );
            ToFrames(new, entry);
        }
    }

    ToFrames( new, DisposeEmptyUSLAB(&old->Slab) );
    return new;
}


Allocator FramesAlloc( Frames* self ) {
    return AllocatorNew(FromFrames, ToFrames, self);
}