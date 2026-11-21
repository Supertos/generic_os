/* Supertos Industries (2012 - 2026) */
#include "RBAllocator.h"


static inline void _ToSelf( RBAllocator* self, Memory in ) {
	RBEntry* cur = FromCaches(self->Caches, sizeof(RBEntry)), *next;
	*cur = (RBEntry){ .Base = in.Base, .End = in.End };
	ToRBTree(&self->Free, cur);
	
	cur = RBTreePrev(&self->Free, cur) ?: cur;
	
	for( usize i = 0; (cur && i < 3); i++ ) {
		next = RBTreeNext(&self->Free, cur);
		
		if( next && next->Base == cur->End ) {
			next->Base = cur->Base;
			ToCache( FromRBTree(&self->Free, cur) );
		}
		
		cur = next;
	}
}


static void ToSelf( RBAllocator* self, Memory in ) {
    ACQUIRE_AUTO(&self->Lock);
	_ToSelf(self, in);
}


static inline usize Score( RBEntry* self, usize size, usize align ) {
	if( self == NULL ) return ~0ULL;
	usize mySize = self->End - min( self->End, AlignUp(self->Base, align) );
	return (mySize < size) ? ~0ULL : (mySize - size);
}


static Memory FromSelf( RBAllocator* self, usize size, usize align, bool first ) {
	ACQUIRE_AUTO(&self->Lock);
	
	RBEntry *cur = NULL, *best = NULL;
	while( (cur = RBTreeNext(&self->Free, cur)) != NULL ) {
		if( Score(best, size, align) >= Score(cur, size, align) ) continue;
		
		best = cur;
		if( Score(best, size, align) == 0 || first ) break;
	}
	
	if( best == NULL ) return MemoryInvalid;
	
	usize oldBase = best->Base, oldEnd = best->End;
	usize base = AlignUp(oldBase, align), end = base + size;
	
	if( oldBase == base ) {
		best->Base = end;
	}else{
		best->End = base;
		if( oldEnd != end ) _ToSelf( self, (Memory){end, oldEnd} );
	}
	
	if( best->Base == best->End ) ToCache( FromRBTree(&self->Free, best) );
	return (Memory){base, end};
}


RBAllocator* RBAllocatorNew( void* begin, Caches* caches ) {
    RBAllocator* self = (RBAllocator*)begin;
    RBTreeNew( &self->Free, offsetof(RBEntry, Index), offsetof(RBEntry, End) );
    self->Caches = caches;
    return self;
}


void RBAllocatorEmpty( RBAllocator* self ) {
	while( RBTreeFirst(&self->Free) != NULL )
		ToCache( FromRBTree(&self->Free, RBTreeFirst(&self->Free)) );
}


void RBAllocatorSetCaches( RBAllocator* self, Caches* caches ) {
	self->Caches = caches;
}


static Memory _FromSelfFirst( RBAllocator* self, usize size, usize align ) {
	return FromSelf(self, size, align, true);
}


static Memory _FromSelfBest( RBAllocator* self, usize size, usize align ) {
	return FromSelf(self, size, align, false);
}


Allocator RBAlloc( RBAllocator* self, bool first ) {
	return AllocatorNew( (first ? _FromSelfFirst : _FromSelfBest), ToSelf, self );
}
