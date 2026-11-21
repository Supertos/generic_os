/* Supertos Industries (2012 - 2025) 
 * Double-linked list with forward insert only.
 */

#pragma once
#include "Base.h"

typedef struct ListIndex ListIndex;
struct ListIndex {
    ListIndex* Prev;
    ListIndex* Next;
};

typedef struct {
    usize First : PHYS_BITS;
    usize IndexOffset : (sizeof(u64) * BITS - PHYS_BITS); 
} List;

void* ToList( List* list, void* item );

void* FromList( List* list, void* item );

void* ListFirst( List* list );

void* ListNext( List* list, void* item );

void* ListPrev( List* list, void* item );

List* ListNew( void* begin, usize indexOffset );