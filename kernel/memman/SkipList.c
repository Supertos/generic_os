/* Supertos Industries (2012 - 2025)
 * Skip List structure implementation.
 * Skip List is used in MemoryRange.c for fast best-fit lookup and on-insert merging.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "SkipList.h"


/* Returns Skip List node height.
 * Node height is either SKIPLIST_MAX_LEVELS or first NULL prev level. (As such, sentinel has height 0)
 * The reason behind not storing height explicitly is saving space to fit SkipList in two cache lines (128 bytes)
 */
static inline size_t SkipListNodeHeight( SkipList* node ) {
    size_t i = 0;
    for( ; i < SKIPLIST_MAX_LEVELS && node->Prev[i]; ++i );
    return i;
}


/* Explicitly initializes SkipList node. */
void SkipListInitNode( SkipList* node, void* payload ) {
    *node = (SkipList){ 0 };
    node->Payload = payload;
}


/* Writes previous nodes on all levels for given weight to given array. */
void SkipListFindPrevs( SkipList* sentinel, SkipList** path, size_t weight ) {
    SkipList* node = sentinel, *next;
    for( ssize_t i = SKIPLIST_MAX_LEVELS - 1; i >= 0; --i ) {
        while( (next=node->Next[i]) && next->Weight < weight ) node = next;
        path[i] = node;
    }
}


/* Inserts node after the given nodes on all levels. */
void SkipListInsertAfter( SkipList** prevs, SkipList* node, size_t height ) {
    if( height >= SKIPLIST_MAX_LEVELS ) {
        height = SKIPLIST_MAX_LEVELS;
    }else{
        node->Next[height] = NULL; // Height is either SKIPLIST_MAX_LEVELS or id of the first NULL element.
        node->Prev[height] = NULL;
    }

    for( size_t i = 0; i < height; ++i ) {
        SkipList* next = prevs[i]->Next[i];
        SkipList* prev = prevs[i];

        node->Next[i] = next;
        prev->Next[i] = node;

        node->Prev[i] = prev;
        if( next ) next->Prev[i] = node;
    }
}


/* Inserts node in Skip List characterized by it's sentinel based on node weight.
 *
 * Sentinel - node with Payload == NULL, usually part of another structure.
 * Caller shall initialize Sentinel with all zeroes to avoid insertion errors.
 */
void SkipListInsert( SkipList* sentinel, SkipList* node, size_t height ) {
    if( !sentinel || !node ) return;
    SkipList* prev[SKIPLIST_MAX_LEVELS];

    SkipListFindPrevs( sentinel, prev, node->Weight );
    SkipListInsertAfter( prev, node, height );
}


/* Removes node from Skip List. */
void SkipListRemove( SkipList* node ) {
    if( !node ) return;
    SkipList** prev = node->Prev;
    SkipList** next = node->Next;

    size_t height = SkipListNodeHeight( node );
    
    for( size_t i = 0; i < height; ++i ) {
        if( prev[i] ) prev[i]->Next[i] = next[i];
        if( next[i] ) next[i]->Prev[i] = prev[i];
    }
}


/* Returns node next element on 'gapeless' level 0. Returns NULL on node == NULL. */
SkipList* SkipListNext( SkipList* node ) {
    if( !node ) return NULL;
    return node->Next[0];
}


/* Returns element previous to node on 'gapeless' level 0. Returns NULL on node == NULL. */
SkipList* SkipListPrev( SkipList* node ) {
    if( !node ) return NULL;
    return node->Prev[0];
}


/* Returns node payload. Payload is pointer to element info. Returns NULL on node == NULL. */
void* SkipListPayload( SkipList* node ) {
    if( !node ) return NULL;
    return node->Payload;
}


/* Sets node payload. Payload is pointer to element info. */
void SetSkipListPayload( SkipList* node, void* payload ) {
    node->Payload = payload;
}


/* Sets node weight. Weight determines node position in list.
 * For example, in address-sorted list (see MemoryRange.c) weight is block address.
 */
void SetSkipListWeight( SkipList* node, size_t weight ) {
    node->Weight = weight;
}


/* Returns best fitting node to given weight. If exact == true returned block is guaranteed to have weight no less than requested.
 * Default (exact == false) behaivour returns block with weight closest to requested. Returns NULL on sentinel == NULL.
 */
SkipList* SkipListBest( SkipList* sentinel, size_t weight, bool exact ) {
    if( !sentinel ) return NULL;
    size_t weightDiff = SIZE_MAX;
    
    SkipList* node = sentinel, *next;
    for( int i = SKIPLIST_MAX_LEVELS - 1; i > 0; --i )  // Fast skip all 'too light' elements until we're at level 0.
        while( (next=node->Next[i]) && next->Weight < weight ) node = next;

    while( (next=node->Next[0]) ) { // Do best-fit on 'gapless' level 0.
        size_t diff = next->Weight > weight ?
                      next->Weight - weight :
                      weight - next->Weight;
        
        if( !exact && diff > weightDiff ) break;

        node = next;
        weightDiff = diff;
        
        if( exact && node->Weight >= weight ) break;
    }

    if( node == sentinel || (exact && weight > node->Weight) ) return NULL;
    return node;
}
