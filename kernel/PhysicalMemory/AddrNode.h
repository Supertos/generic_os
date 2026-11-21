/* Supertos Industries (2012 - 2026) */
#pragma once

#include "Frames.h"

static inline AddrNode* AddrNodeNew( AddrNode* self, u64 addr ) {
    *self = (AddrNode){ .Key = addr };
    
    U64CacheNew(&self->Cache, ADDR_NODE_ELMS);
    return self;
}

static inline void AddrNodeTransfer( AddrNode* self, AddrNode* other, u64 threshold ) {
    U64Stack *from = &self->Cache, *to = &other->Cache;

    for( usize i = 0; i < from->Count; i++ ) {
        while( from->Items[i] >= threshold ) {
            u64 framePtr = FromU64Cache(&from->Items[i]);
            Frame* val = (Frame*)framePtr;
            val->OwnerPtr = ToU64Cache(to, (u64)val);
        }
    }
}

static inline u64 NodeCount( AddrNode* self ) {
    return (self == NULL) ? ~0ULL : self->Cache.Count;
}

static inline AddrNode* SplitAddrNode( AddrNode* self, Frames* frames, Memory* emrg ) {
    AddrNode* new = FromUSLAB(&frames->Slab, emrg);
    if( new == NULL ) return NULL;
    u64 median = (self == NULL) ? 0 : U64CacheMedian(&self->Cache);
    AddrNodeNew(new, median);

    if( self != NULL ) AddrNodeTransfer(self, new, median);
    ToRBTree( &frames->AddrTree, new );
    return new;
}

static inline bool TryMergeAddrNode( AddrNode* self, Frames* frames ) {
    RBTree* tree = &frames->AddrTree;

    AddrNode *prev = RBTreePrev(tree, self), *next = RBTreeNext(tree, self);
    if( prev == NULL && next == NULL ) return false;

    bool prevLeast = NodeCount(prev) < NodeCount(next);
    AddrNode* merge = prevLeast ? prev : next;
    
    if( merge->Cache.Count + NodeCount(self) >= ADDR_NODE_ELMS ) return false;
    AddrNodeTransfer(merge, self, 0);

    FromRBTree( tree, merge );
    ToUSLAB( &frames->Slab, merge );
    return true;
}

static inline u64* ToAddrTree( Frames* frames, Frame* self, Memory* emrg ) {
    AddrNode* node = SearchRBTreeFloor(&frames->AddrTree, (u64)self);
    
    if( node == NULL || NodeCount(node) >= U64CACHE_ALIGN )
        node = SplitAddrNode(node, frames, emrg);

    if( node == NULL || MemorySize(*emrg) == 0 ) return NULL;
    self->OwnerPtr = ToU64Cache(&node->Cache, (u64)self);
    return self->OwnerPtr;
}

static inline Frame* FromAddrTree( Frames* frames, Frame* self ) {
    u64*      val   = (u64*)self->OwnerPtr;
    AddrNode* node  = (AddrNode*)ItemU64Cache(val);
    
    FromU64Cache(val);

    if( *val != (u64)NULL && NodeCount(node) > U64ItemID(val) )
        ((Frame*)*val)->OwnerPtr = val;

    if( NodeCount(node) == 0 ) {
        FromRBTree(&frames->AddrTree, node);
        ToUSLAB(&frames->Slab, node);
    }else if( NodeCount(node) <= ADDR_NODE_ELMS * 0.3 ) {
        TryMergeAddrNode(node, frames);
    }
    return self;
}

static inline U64Result SearchAddrTree( Frames* self, u64 key ) {
    RBTree*   tree = &self->AddrTree;
    AddrNode* cur  = SearchRBTreeFloor(tree, key);
    U64Result out  = (cur == NULL) ? (U64Result){ NULL, NULL, NULL }
                                   : QueryU64Cache(&cur->Cache, key);

    AddrNode *next = NULL, *prev = NULL;
    if( out.Next == NULL && (next = RBTreeNext(tree, cur)) != NULL )
        out.Next = QueryU64Cache(&next->Cache, key).Next;

    if( out.Prev == NULL && (prev = RBTreePrev(tree, cur)) != NULL )
        out.Prev = QueryU64Cache(&prev->Cache, key).Prev;

    return out;
}

static inline void UpdateAddrReference( u64* ownerPtr, u64 newValue ) {
    AddrNode* self = (AddrNode*)ItemU64Cache(ownerPtr);
    self->Key = min(self->Key, newValue);
    *ownerPtr = newValue;
}