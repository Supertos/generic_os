/* Supertos Industries (2012 - 2026) */
#include "Cache.h"

#define FREE false
#define USED true
#define USED_WHOLE (char)(~0ULL)

typedef struct {
    u16 BinCount;
	u16 BinSize;
	u16 Synchro;
	u16 Present;
    void* Owner;
	ListIndex Index;
	Bitmap Slots[];
} SLAB;


static inline usize Slots( usize binSize ) {
	return PAGE / binSize;
}


static inline usize MaxSlots( usize binSize ) {
	return (PAGE - AlignUp(sizeof(SLAB) + sizeof(Bitmap) * BITMAP_SIZE(Slots(binSize)), CACHELINE)) / binSize;
}


static SLAB* SLABNew( void* begin, usize binSize, void* owner ) {
	SLAB* self  = begin;
	usize slots = Slots(binSize), maxSlots = MaxSlots(binSize);
    *self 		= (SLAB){ .BinCount = slots, .BinSize = binSize, .Owner = owner };

    MemorySet( MemoryNew(self->Slots, BITMAP_SIZE(slots) * sizeof(Bitmap)), USED_WHOLE ); // BITMAP_SIZE(slots) * slots_per_bitmap >= slots
	SetBits( self->Slots, slots - maxSlots, slots, FREE );

    return self;
}


static void* FromSLAB( SLAB* self ) {
    if( self->BinCount == 0 ) return NULL;
	
	usize slot = TrySetBitsAtomic( self->Slots, BITMAP_SIZE(Slots(self->BinSize)), 1 );
	if( slot == NOID ) return NULL;
	FetchSub16(&self->BinCount, 1);
	
	
	return (void*)((usize)self + slot * self->BinSize);
}


static void ToSLAB( void* addr ) {
    if( addr == NULL ) return;
	
	SLAB* self = (SLAB*)AlignDown(addr, PAGE);
	usize slot = ((usize)addr % PAGE) / self->BinSize;
	
	SetBits( self->Slots, slot, 1, FREE );
    FetchAdd16(&self->BinCount, 1);
}


static void UpdateSLAB( Cache* self, SLAB* slab ) {
	if( FetchAdd16( &slab->Synchro, 1 ) > 0 ) return;
	ACQUIRE_AUTO(&self->InUse);
	
	bool empty = (slab->BinCount == 0), full = (slab->BinCount == MaxSlots(slab->BinSize));
	bool mustList = !empty && !full, mustFree = empty;
	
	if( mustList != slab->Present ) {
		mustList ? ToList(&self->Fragments, slab) : FromList(&self->Fragments, slab);
		slab->Present = mustList;
	}
	
	slab->Synchro = 0;
	architecture_barrier;
	compiler_barrier;
	if( mustFree ) To( self->Source, MemoryNew(slab, PAGE) );
}


static SLAB* NewCacheSLAB( Cache* self ) {
	Memory mem = self->Source.From( self->Source.Self, PAGE, PAGE );
	if( !MemoryValid(mem) ) return NULL;
	return SLABNew((void*)mem.Base, self->BinSize, self);
}


void* FromCache( Cache* self ) {
	RWLock* lock = &self->InUse;
	void* out = NULL;
	while( out == NULL ) {
		AcquireReader(lock);
		SLAB* cur = ListFirst(&self->Fragments);

		if( cur == NULL ) {
			ReleaseReader(lock);
			// There is indeed no protection from multiple cores creating new SLABs.
			// It's intended, think of this as high-load caching.
			cur = NewCacheSLAB(self);
			if( cur == NULL ) return NULL;
			AcquireReader(lock);
		}
		
		bool wasFull = (cur->BinCount == MaxSlots(cur->BinSize));
		out = FromSLAB(cur);
		bool empty = (cur->BinCount == 0);
		bool willUpdate = (wasFull || empty);
		ReleaseReader(lock);
		
		if( willUpdate ) UpdateSLAB(self, cur);
	}

	return out;
}


void ToCache( void* addr ) {
	SLAB* slab = (SLAB*)AlignDown(addr, PAGE);
	Cache* self = slab->Owner;
	AcquireReader(&self->InUse);
	
	bool wasEmpty = (slab->BinCount == 0);
	ToSLAB(addr);
	bool full = (slab->BinCount == MaxSlots(slab->BinSize));
	
	bool willUpdate = (wasEmpty || full);
	ReleaseReader(&self->InUse);
	if( willUpdate ) UpdateSLAB(self, slab);
}


Cache* CacheNew( void* base, Allocator source, usize size ) {
	if( size == 0 ) goto cacheNew_hard;

	u16 binSize = (u16)(1ULL << (1 - LZCNT64((u64)size)));
	if( binSize < size ) binSize *= 2;
	if( binSize >= PAGE || binSize == 0 ) goto cacheNew_hard;

	Cache* self = base;
	*self = (Cache){ .BinSize = binSize, .Source = source };
	ListNew( &self->Fragments, offsetof(SLAB, Index) );
	return self;

	[[gnu::cold]] cacheNew_hard:
		KernelCrashUndBurn("Bad cache size");
		return NULL;
}


Caches* CachesNew( void* base, Allocator source ) {
	Caches* self = (Caches*)base;
	for( usize i = 0; i < CACHE_SIZES; i++ )
		CacheNew( &self->Sizes[i], source, CACHE_MIN_SIZE *(1 << (i)) );

	return self;
}

u64 CacheSize() {
	return sizeof(Cache);
}

u64 CachesSize() {
	return sizeof(Caches);
}

void* FromCaches( Caches* self, usize size ) {
	usize id = TZCNT64((u64)size / CACHE_MIN_SIZE);
	if( (1ULL << id) > size ) id++;

	return FromCache(&self->Sizes[id / CACHE_MIN_SIZE]);
}