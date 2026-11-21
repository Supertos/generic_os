/* Supertos Industries (2012 - 2026) */
#include "List.h"
#include "Base.h"


static inline ListIndex* Index( List* list, void* item ) {
    if( item == NULL ) return NULL;
    return (ListIndex*)((char*)item + list->IndexOffset);
}


static inline void* Item( List* list, ListIndex* index ) { 
    if( index == NULL ) return NULL;
    return (char*)index - list->IndexOffset;
}


void* ToList( List* list, void* item ) {
    ListIndex* index = Index(list, item), *first = ListFirst(list);
    *index = (ListIndex){ .Next = first, .Prev = NULL };
    
    if( first != NULL ) first->Prev = index;
    list->First = (usize)index;
    return item;
}


void* FromList( List* list, void* item ) {
    ListIndex* index = Index(list, item);
    ListIndex *next = index->Next, *prev = index->Prev;

    if( next != NULL ) next->Prev = prev;

    if( prev != NULL )
        prev->Next = next;
    else
        list->First = (usize)next;
    return item;
}


void* ListFirst( List* list ) {
    if( (u64)list->First == (1ULL << PHYS_BITS) - 1 )
        return NULL;
    return Item(list, (ListIndex*)(usize)list->First);
}


void* ListNext( List* list, void* item ) {
    if( item == NULL ) return ListFirst(list);
    
    ListIndex* index = Index(list, item);
    return Item(list, index->Next);
}


void* ListPrev( List* list, void* item ) {
    if( item == NULL ) return NULL;
    
    ListIndex* index = Index(list, item);
    return Item(list, index->Prev);
}


List* ListNew( void* begin, usize indexOffset ) {
    List* list = begin;
    *list = (List){ .IndexOffset = indexOffset, .First = (u64)NULL & MASK(u64, 0, PHYS_BITS) };
    return list;
}