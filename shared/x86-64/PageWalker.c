/* Supertos Industries (2012 - 2026) */

#include "PageWalker.h"


#define PT_ENTRIES (u64)(1 << LEVEL_EXP)
#define PT_ALIGN (u64)(1 << PAGE_EXP)

#define ATTRIBUTE_MASK (MASK(u64, 0, 12) | MASK(u64, 63, 1))

#define PTE_PHYS (PHYS_BITS - PAGE_EXP)

struct __attribute__((__may_alias__)) PTE{
    u64 Present : 1;
    u64 Writable : 1;
    u64 User : 1;
    u64 WriteThrough : 1;
    u64 CacheDisable : 1;
    u64 Accessed : 1;
    
    u64 Written : 1;
    u64 Huge : 1;
    u64 Global : 1;
    u64 __Reserved1 : 3;

    u64 PAddr: PTE_PHYS;
    u64 __Available : 12;

    u64 DisableExecution : 1;
} ;


void* PTEAddr( PTE* entry ) { 
    if( entry == NULL || entry->Present == 0 ) return NULL;
    return (void*)(u64)(entry->PAddr << PAGE_EXP); 
}


static inline PTE* SetPTE( PTE* pte, void* addr, char* mode ) {
    *pte = (PTE) { .__Available = pte->__Available, .User = 1, .DisableExecution = 1, .PAddr = ((u64)addr >> PAGE_EXP) };
	
    for( u64 i = 0; mode && mode[i]; i++ ) {
		switch( mode[i] ) {
			case 'R': pte->Present = 1; break;
			case 'W': pte->Writable = 1; break;
			case 'X': pte->DisableExecution = 0; break;

			case 'N': pte->CacheDisable = 1; break; // Non-cacheable
			case 'S': pte->WriteThrough = 1; break; // Shared

			case 'G': pte->Global = 1; break;
			case 'K': pte->User = 0; break; // Kernel

			case 'H': pte->Huge = 1; break;

			default: break;
		}
    }

    return pte;
}


typedef struct {
	PTE* Entry;
	usize Base;
	usize ID;
} VMLevel;


static inline VMLevel* CurLevel( PageWalker* self ) {
    return (VMLevel*)(self->Stack + self->Level * sizeof(VMLevel));
}


usize WalkerStep( PageWalker* self ) {
	usize shift = PAGE_EXP + LEVEL_EXP * self->Level;
	if( shift >= bitsof(usize) ) return ~0ULL;
    return 1ULL << shift;
}


usize WalkerAddress( PageWalker* self ) {
	VMLevel* cur = CurLevel(self);
	return cur->Base + cur->ID * WalkerStep(self);
}


static inline bool WalkerCanAscend( PageWalker* self ) {
	return self->Level < VM_LEVELS - 1;
}


static inline bool WalkerCanDescend( PageWalker* self, u16 id ) {
	VMLevel* cur = CurLevel(self);
	return self->Level > 0 && (cur->Entry == NULL || !cur->Entry[id].Huge || !cur->Entry->Present);
}


static inline u16 AddToCounter( PTE* pte, u16 delta ) {
    PTE* first = (PTE*)AlignDown((usize)pte, PT_ALIGN);
	first->__Available += delta;
    return first->__Available;
}


static inline VMLevel* WalkerDescend( PageWalker* self, u8 id ) {
    VMLevel* old = CurLevel(self);
	usize newBase = old->Base + id * WalkerStep(self);
    old->ID = id;
    if( !WalkerCanDescend(self, id) ) return NULL;
	
	if( PTEAddr(&old->Entry[id]) == NULL && self->Source.From != NULL ) {
		void* data = (void*)self->Source.From( self->Source.Self, PAGE, PT_ALIGN ).Base;
		WalkerSetEntry( self, PTNew(data), "RWNSK" );
	}
	
	self->Level--;
	*CurLevel(self) = (VMLevel) {
		.Entry = (PTE*)PTEAddr(&old->Entry[id]),
		.ID = -1,
		.Base = newBase
	};
	
	return CurLevel(self);
}


static inline VMLevel* WalkerAscend( PageWalker* self ) {
    self->Level++;
    return CurLevel(self);
}


usize WalkerCurAddress( PageWalker* self ) {
	return CurLevel(self)->Base + CurLevel(self)->ID * WalkerStep(self);
}


PTE* WalkerNext( PageWalker* self ) {
	VMLevel* top = CurLevel(self);

	while( top->ID == PT_ENTRIES - 1 && WalkerCanAscend(self) ) // (usize)-1 == ~0ULL shall not pass this one
        top = WalkerAscend(self);
	
	if( top->ID == PT_ENTRIES - 1 ) return NULL;
	
	while( WalkerCanDescend(self, top->ID + 1) )
		top = WalkerDescend(self, top->ID + 1);
	
	top->ID++;
	return top->Entry ? &top->Entry[top->ID] : NULL;
}


PTE* WalkerSetEntry( PageWalker* self, void* addr, char* mode ) {
	VMLevel* top = CurLevel(self);
	PTE* pte = &top->Entry[top->ID];
    bool changed = (PTEAddr(pte) != NULL) != (addr != NULL);
    SetPTE( pte, addr, mode );
    if( !changed ) return pte;

    u16 counter = AddToCounter(pte, addr ? 1 : -1);
    if( counter > 0 || !WalkerCanAscend(self) || self->Source.To == NULL ) return pte;
	
	To( self->Source, MemoryNew(AlignDown((usize)pte, PT_ALIGN), PAGE) );
    WalkerAscend(self);
    tail return WalkerSetEntry(self, NULL, "");
}


void WalkerSetHuge( PageWalker* self ) {
	CurLevel(self)->Entry[CurLevel(self)->ID].Huge = 1;
}


PTE* WalkerEntry( PageWalker* self, usize vAddr, u64 align ) {
    u64 level, id;
	switch( align ) {
		case HUGEPAGE: level = 1; break;
		case GIGAPAGE: level = 2; break;
		default: level = 0; break;
	}
	
	self->Level = VM_LEVELS - 1;
	do {
		id = vAddr / WalkerStep(self);
		vAddr %= WalkerStep(self);
	} while( level < self->Level && WalkerDescend(self, id) );
	return &CurLevel(self)->Entry[id];
}


static inline PTE* WalkerSkipNULLsAtLevel( PageWalker* self ) {
	VMLevel* top = CurLevel(self);
	if( top->Entry == NULL ) return NULL;
	
	while( ++top->ID < PT_ENTRIES ) {
		PTE* cur = &top->Entry[top->ID];
		if( PTEAddr(cur) != NULL ) return cur;
	}
	
	return NULL;
}


PTE* WalkerSkipNULLs( PageWalker* self, usize maxVAddr ) {
	while( WalkerCurAddress(self) < maxVAddr ) {
		PTE* out = WalkerSkipNULLsAtLevel(self);
		VMLevel* top = CurLevel(self);
		
		if( PTEAddr(out) != NULL ) {
			if( !WalkerCanDescend(self, top->ID) ) return out;
			WalkerDescend( self, top->ID );
			continue;
		}
		
		if( !WalkerCanAscend(self) ) break;
		WalkerAscend(self);
	}
    return NULL;
}


PageWalker WalkerNew( void* Root, Allocator allocator ) {
	PageWalker self = (PageWalker){ 
        .Level = VM_LEVELS - 1, .Source = allocator
    };

	*CurLevel(&self) = (VMLevel){ .Entry = Root, .ID = -1, .Base = 0 };
	return self;
}


PTE* WalkerCurEntry( PageWalker* self ) {
	return &CurLevel(self)->Entry[CurLevel(self)->ID];
}