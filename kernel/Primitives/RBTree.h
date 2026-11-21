/* Supertos Industries (2012 - 2026) 
 * Intrusive Red-Black Tree container.
 * RBIndex interpretes MSB of parent pointer as color, since no modern MMU supports 64-bit addressing and kernel always uses zeroed upper bits.
 *
 * TODO: I'm not writing down all theoretical foundations of RB-Tree. See Linux implementation (they even have ASCII graphics for fixups!) or Wikipedia.
 * Except their code uses explicit mirroring and fixups are iterative (this implementation uses recursion with forced tail optimization (via [[gnu::musttail]])).
 * https://github.com/torvalds/linux/blob/master/lib/rbtree.c - Linux RB-Tree
 */

#pragma once

#include "Base.h"

typedef struct RBIndex RBIndex;
struct RBIndex {
    u64      Parent : 63;
    u64      Color  : 1;
    RBIndex* Left;
    RBIndex* Right;
};

typedef struct {
    RBIndex* Root;
    u32      IndexOffset;
    u32      ValueOffset;
    RBIndex  _NIL;
} RBTree;

void* SearchRBTree( RBTree* tree, u64 key );

void* FromRBTree( RBTree* tree, void* element );

void ToRBTree( RBTree* tree, void* element );

void* RBTreeRoot( RBTree* tree );

void* RBTreeFirst( RBTree* tree );

void* RBTreeNext( RBTree* tree, void* item );

void* RBTreePrev( RBTree* tree, void* item );

RBTree* RBTreeNew( void* begin, u16 indexOffset, u16 valueOffset );

void* SearchRBTreeFloor( RBTree* tree, u64 key );