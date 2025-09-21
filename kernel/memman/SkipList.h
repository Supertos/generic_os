/* Supertos Industries (2012 - 2025)
 * Skip List structure implementation.
 * Skip List is used in MemoryRange.c for fast best-fit lookup and on-insert merging.
 */
#pragma once
#include <stdint.h>
#include <stdbool.h>

#define SKIPLIST_MAX_LEVELS 7


typedef struct SkipList SkipList; 
struct SkipList {
    size_t Weight;
    SkipList* Next[SKIPLIST_MAX_LEVELS];
    // There shall be cache line border
    void* Payload;
    SkipList* Prev[SKIPLIST_MAX_LEVELS];
} __attribute__((__packed__));


/* Explicitly initializes SkipList node. */
void SkipListInitNode( SkipList* node );


/* Inserts node in Skip List characterized by it's sentinel based on node weight.
 *
 * Sentinel - node with Payload == NULL, usually part of another structure.
 * Caller shall initialize Sentinel with all zeroes to avoid insertion errors.
 */
void SkipListInsert( SkipList* sentinel, SkipList* node, size_t height );


/* Removes node from Skip List. */
void SkipListRemove( SkipList* node );


/* Returns node next element on 'gapeless' level 0. Returns NULL on node == NULL. */
SkipList* SkipListNext( SkipList* node );


/* Returns element previous to node on 'gapeless' level 0. Returns NULL on node == NULL. */
SkipList* SkipListPrev( SkipList* node );


/* Returns node payload. Payload is pointer to element info. Returns NULL on node == NULL. */
void* SkipListPayload( SkipList* node );


/* Sets node payload. Payload is pointer to element info. */
void SetSkipListPayload( SkipList* node, void* payload );


/* Sets node weight. Weight determines node position in list.
 * For example, in address-sorted list (see MemoryRange.c) weight is block address.
 */
void SetSkipListWeight( SkipList* node, size_t weight );


/* Returns best fitting node to given weight. If exact == true returned block is guaranteed to have weight no less than requested.
 * Default (exact == false) behaivour returns block with weight closest to requested. Returns NULL on sentinel == NULL.
 */
SkipList* SkipListBest( SkipList* sentinel, size_t weight, bool exact );