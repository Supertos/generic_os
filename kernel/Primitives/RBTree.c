/* Supertos Industries (2012 - 2026) */
#include "RBTree.h"
#include "Base.h"


#define RED 0
#define BLACK 1

#define LEFT true
#define RIGHT false


#define NIL(tree) &tree->_NIL


typedef struct {
    RBIndex* Result;
    RBIndex* Parent;
    RBIndex* Closest;
} SearchResult;


static inline RBIndex* Parent( RBIndex* index ) { 
    return (RBIndex*)(usize)index->Parent; 
}


static inline RBIndex* Grand( RBIndex* index ) { 
    return Parent( Parent(index) );
}


static inline bool IsLeftChild( RBIndex* index ) {
    return Parent(index)->Left == index; 
}


static inline RBIndex** Child( RBIndex* index, bool leftPredicat ) {
    return leftPredicat ? &index->Left : &index->Right;
}


static inline RBIndex* Sibling( RBIndex* index ) {
    return *Child( Parent(index), !IsLeftChild(index) ); 
}


static inline RBIndex* Uncle( RBIndex* index ) {
    return Sibling( Parent(index) ); 
}


static inline bool IsNil( RBIndex* index ) { 
    return Parent(index) == index; 
}


static inline void SetBlack( RBIndex* index ) {
    index->Color = BLACK;
}


static inline void SetRed( RBIndex* index ) {
    if( !IsNil(index) ) index->Color = RED; 
}


static inline bool IsBlack( RBIndex* index ) { 
    return index->Color == BLACK; 
}


static inline bool IsRed( RBIndex* index ) { 
    return index->Color == RED; 
}


static inline void SetParent( RBIndex* index, RBIndex* parent ) { 
    if( !IsNil(index) ) index->Parent = (usize)parent; 
}


static inline void* Item( RBTree* tree, RBIndex* index ) {
    if( IsNil(index) ) return NULL;
    return (char*)index - tree->IndexOffset; 
}


static inline RBIndex* Index( RBTree* tree, void* item ) {
    return (RBIndex*)( (usize)item + tree->IndexOffset );
}


static inline u64* Value( RBTree* self, RBIndex* index ) {
    return (u64*)((char*)Item(self, index) + (usize)self->ValueOffset);
}


static inline void Attach( RBTree* tree, RBIndex* index, RBIndex* newParent, bool isLeft ) {
    if( (usize)newParent == (usize)index ) return;
    RBIndex **updateTarget = Child(newParent, isLeft), *exParent = Parent(index);

    if( IsNil(newParent) ) updateTarget = &tree->Root;
    if( !IsNil(exParent) ) *Child(exParent, IsLeftChild(index)) = NIL(tree);
    
    SetParent( index, newParent );
    SetParent( *updateTarget, NIL(tree) );
    *updateTarget = index;
}


static inline void Rotate( RBTree* tree, RBIndex* index, bool isLeft ) {
    RBIndex *child = *Child(index, !isLeft), *grandChild = *Child(child, isLeft);

    Attach( tree, grandChild, index, !isLeft );
    Attach( tree, child, Parent(index), IsLeftChild(index) );
    Attach( tree, index, child, isLeft );
}


static inline SearchResult NodeByKey( RBTree* tree, u64 key ) {
    RBIndex *node = tree->Root, *parent = NIL(tree), *closest = NIL(tree);
    while( !IsNil(node) ) {
        int compare = *Value(tree, node) > key;
        if( compare <= 0 ) closest = node;
        if( compare == 0 ) break;

        parent = node;
        node = *Child(parent, compare > 0);
    }

    return (SearchResult){ .Result = node, .Parent = parent, .Closest = closest };
}


static void InsertFixup( RBTree* tree, RBIndex* index ) {
    RBIndex *grand = Grand(index), *parent = Parent(index), *uncle = Uncle(index);
    if( IsNil(parent) ) SetBlack(index);
    if( IsBlack(parent) ) return;

    if( IsRed(uncle) ) {
        SetBlack(parent);
        SetBlack(uncle);
        SetRed(grand);
        tail return InsertFixup(tree, grand);
    }

    bool zigzag = IsLeftChild( Parent(index) ) != IsLeftChild(index);
    bool isRight = !IsLeftChild(index);
    if( !zigzag ) {
        Rotate( tree, grand, isRight );
        SetBlack(parent);
        SetRed(grand);
        return;
    }

    Rotate( tree, parent, isRight );
    tail return InsertFixup(tree, parent);
}


static void DeleteFixup( RBTree* tree, RBIndex* index, RBIndex* parent ) {
    if( (usize)index == (usize)tree->Root ) {
        SetBlack(index);
        return;
    }
    
    bool left = (parent->Left == index);
    RBIndex* sibling = *Child( parent, !left );
    RBIndex *nearCousin = *Child(sibling, left), *farCousin = *Child(sibling, !left);

    if( IsRed(sibling) ) {
        SetBlack(sibling);
        SetRed(parent);
        Rotate(tree, parent, left);
        tail return DeleteFixup(tree, index, parent);
    }

    if( IsBlack(nearCousin) && IsBlack(farCousin) ) {
        SetRed(sibling);

        if( IsBlack(parent) ) DeleteFixup( tree, parent, Parent(parent) );
        SetBlack(parent);
        return;
    }
    
    if( IsRed(nearCousin) && IsBlack(farCousin) ) {
        SetBlack(nearCousin);
        SetRed(sibling);
        Rotate(tree, sibling, !left);
        tail return DeleteFixup(tree, index, parent);
    }
    
    if( !IsRed(farCousin) ) return;

    sibling->Color = parent->Color;
    SetBlack(parent);
    SetBlack(farCousin);
    Rotate(tree, parent, left);
}


void ToRBTree( RBTree* tree, void* item ) {
    RBIndex* index = (RBIndex*)((char*)item + tree->IndexOffset);
    u64* valueAddr = Value(tree, index);
    
    SearchResult insertInfo = NodeByKey( tree, *valueAddr );
    if( !IsNil(insertInfo.Result) ) return;

    *index = (RBIndex){ .Color = RED, .Left = NIL(tree), .Right = NIL(tree), .Parent = (usize)NIL(tree) };

    int comparator = 0;
    if( !IsNil(insertInfo.Parent) ) comparator = *Value(tree, insertInfo.Parent) > *valueAddr;
    Attach( tree, index, insertInfo.Parent, comparator > 0 );

    InsertFixup( tree, index );
}


static inline void Swap( RBTree* tree, RBIndex* a, RBIndex* b ) {
    RBIndex temp = *b;
    bool isALeft = IsLeftChild(a), isBLeft = IsLeftChild(b);
    bool cpA = Parent(a) == b, cpB = Parent(b) == a;

    Attach( tree, a->Left, b, LEFT );
    Attach( tree, a->Right, b, RIGHT );
    Attach( tree, temp.Left, a, LEFT ); 
    Attach( tree, temp.Right, a, RIGHT );

    Attach( tree, b, cpA ? a : Parent(a), isALeft );
    Attach( tree, a, cpB ? b : Parent(&temp), isBLeft );

    b->Color = a->Color;
    a->Color = temp.Color;
}


void* FromRBTree( RBTree* tree, void* item ) {
    RBIndex* index = Index(tree, item);
    SearchResult removeInfo = NodeByKey( tree, *Value(tree, index) );
    if( IsNil(removeInfo.Result) ) return NULL;

    usize children = (index->Left != NIL(tree)) + (index->Right != NIL(tree));
    if( children == 2 ) {
        RBIndex* successor = index->Right;
        while( !IsNil(successor->Left) ) successor = successor->Left;
        Swap( tree, index, successor );
    }

    if( (usize)tree->Root == (usize)index && children == 0 ) {
        tree->Root = NIL(tree);
        return item;
    }

    RBIndex *child = *Child( index, !IsNil(index->Left) ), *parent = Parent(index);
    Attach( tree, child, parent, IsLeftChild(index) );

    if( IsBlack(index) ) 
        DeleteFixup(tree, child, parent);
    return item;
}


void* SearchRBTree( RBTree* tree, u64 key ) {
    RBIndex* index = NodeByKey(tree, key).Result;
    return index == NIL(tree) ? NULL : Item(tree, index);
}


void* SearchRBTreeFloor( RBTree* tree, u64 key ) {
    RBIndex* index = NodeByKey(tree, key).Closest;
    return index == NIL(tree) ? NULL : Item(tree, index);
}


void* RBTreeRoot( RBTree* tree ) {
    return IsNil(tree->Root) ? NULL : Item(tree, tree->Root);
}


void* RBTreeFirst( RBTree* tree ) {
    RBIndex* cur = tree->Root;
    while( !IsNil(cur->Left) ) cur = cur->Left;
    return IsNil(cur) ? NULL : Item(tree, cur);
}


void* RBTreeNext( RBTree* tree, void* item ) {
    if( item == NULL ) return RBTreeFirst(tree);
    RBIndex* index = Index(tree, item);
    RBIndex* out = index->Right;

    if( !IsNil(out) ) {
        while( !IsNil(out->Left) ) out = out->Left;
    }else{
        out = index;
        while( !IsNil(out) && !IsLeftChild(out) ) out = Parent(out);
		out = Parent(out);
    }

    return Item(tree, out);
}


void* RBTreePrev( RBTree* tree, void* item ) {
    if( item == NULL ) return NULL;
    RBIndex* index = Index(tree, item);
    RBIndex* out = index->Left;

    if( !IsNil(out) ) {
        while( !IsNil(out->Right) ) out = out->Right;
    }else{
        out = index;
        while( !IsNil(out) && IsLeftChild(out) ) out = Parent(out);
		out = Parent(out);
    }

    return Item(tree, out);
}


RBTree* RBTreeNew( void* begin, u16 indexOffset, u16 valueOffset ) {
    RBTree* tree = begin;

    *tree = (RBTree) {
        .IndexOffset = indexOffset, .ValueOffset = valueOffset,
        .Root = &tree->_NIL,
        ._NIL = { .Color = BLACK, .Parent = (usize)(&tree->_NIL), .Left = &tree->_NIL, .Right = &tree->_NIL }
    };
    return tree;
}