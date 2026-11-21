/* Supertos Industries (2012 - 2026) 
 * May be modified to handle concurrent reads and modifications.
 * #define U64_PENDING *some invalid value*
 * Thread acquires elements by CAS'ing it to U64_PENDING.
 * Then make U64Stack->Items[self->Count - 1] into spinlock:
 * To change element count you must CAS U64Stack->Items[self->Count - 1] to U64Pending.
 * Must not cross U64CACHE_ALIGN boundary.
 */
#pragma once
#include "Base.h"

#define U64CACHE_ALIGN (CACHELINE * 4)

typedef struct {
    u16 Count;
    u16 Capacity;
    u16 Map;
    u64 Items[];
} U64Stack;

typedef struct {
    u64* Prev;
    u64* Trg;
    u64* Next;
} U64Result;

static inline U64Stack* ItemU64Cache( void* el ) {
    return (U64Stack*)AlignDown((usize)el, U64CACHE_ALIGN);
}

static inline u16 U64ItemID( void* el ) {
    U64Stack* self = ItemU64Cache(el);
    return idof(self->Items, el);
}

static inline u64* ToU64Cache( U64Stack* self, u64 val ) {
    if( self->Count >= self->Capacity ) return NULL;
    u64* out = &self->Items[self->Count++];
    *out = val;

    return out;
}

static inline u64 FromU64Cache( u64* val ) {
    U64Stack* self = ItemU64Cache(val);

    u64 out = *val;
    *val = self->Items[--self->Count];
    if( val == &self->Items[self->Count] ) *val = (u64)NULL;

    return out;
}

static inline u64 U64CacheMedian( U64Stack* self ) {
    u64 val = 0;
    for( usize i = 0; i < self->Count; i++ ) val += self->Items[i];
    val /= self->Count;
    return val;
}

static inline U64Result QueryU64Cache( U64Stack* self, u64 key ) {
    u64 *prev = NULL, *next = NULL, *trg = NULL;
    for( usize i = 0; i < self->Count; i++ ) {
        u64* cur = &self->Items[i];

        if( *cur < key && (prev == NULL || *cur > *prev) ) prev = cur;
        if( *cur > key && (next == NULL || *cur < *next) ) next = cur;
        if( *cur == key ) trg = cur;
    }

    return (U64Result){prev, trg, next};
}

static inline U64Stack* U64CacheNew( void* base, u16 capacity ) {
    U64Stack* self = (U64Stack*)base;
    *self = (U64Stack){ .Capacity = capacity };

    return self;
}

static inline u64* U64CacheTop( U64Stack* self ) {
    return &self->Items[self->Count - 1];
}