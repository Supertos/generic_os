/* Supertos Industries (2012 - 2026) */
#pragma once

#include "Base.h"
#include "Primitives/List.h"
#include "Primitives/RWLock.h"
#include "MemoryManager.h"

#define BRB_SLOT_SIZE   (CACHELINE * 4)
#define BRB_SLOTS       ((PAGE - sizeof(BRBCache) ) / BRB_SLOT_SIZE)

#define BRB_EMPTY_SLOTS ((~MASK(u64, 1, BRB_SLOTS)))
#define BRB_FULL_SLOTS  (~0ULL)

typedef struct {
    u64 Slots;
    ListIndex Index;
} BRBCache;

typedef struct {
    List Caches;
    void* Owner;
} USLAB;

static inline void* FromUSLAB( USLAB* self, Memory* emrg ) {
    BRBCache* cur = ListFirst(&self->Caches);

    if( cur == NULL ) {
        emrg->End -= PAGE;
        cur = (BRBCache*)emrg->End;
        cur->Slots = BRB_EMPTY_SLOTS;
        ToList(&self->Caches, cur);
    }

    if( !MemoryValid(*emrg) ) return NULL;

    usize id = TZCNT64(~cur->Slots);

    void* out = ((char*)cur + BRB_SLOT_SIZE * id);
    cur->Slots |= (1ULL << id);

    if( cur->Slots == BRB_FULL_SLOTS ) FromList(&self->Caches, cur);
    return out;
}

static inline void ToUSLAB( USLAB* self, void* val ) {
    BRBCache* owner = (BRBCache*)AlignDown((usize)val, PAGE);
    if( owner->Slots == BRB_FULL_SLOTS ) 
        ToList(&self->Caches, owner);

    usize id = ((usize)val - (usize)owner) / BRB_SLOT_SIZE;
    owner->Slots &= ~(1ULL << id);

    if( owner->Slots == BRB_EMPTY_SLOTS ) {
        FromList(&self->Caches, owner);
        ToFramesNoLock( self->Owner, MemoryNew(owner, PAGE) );
        return;
    }
}

static inline Memory DisposeEmptyUSLAB( USLAB* self ) {
    BRBCache* cache = FromList(&self->Caches, ListFirst(&self->Caches));
    return (cache != NULL) ? MemoryNew((u64)cache, PAGE) : MemoryInvalid;
}

static inline USLAB* USLABNew( void* base, void* owner ) {
    USLAB* self = (USLAB*)base;
    *self = (USLAB){ .Owner = owner };
    ListNew( &self->Caches, offsetof(BRBCache, Index) );
    return self;
}