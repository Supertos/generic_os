/* Supertos Industries (2012 - 2026) */
#pragma once

#include "Base.h"
#include "MemoryManager.h"
#include "Primitives/RBTree.h"
#include "Primitives/RWLock.h"
#include "Primitives/List.h"
#include "USLAB.h" 
#include "U64Cache.h"


#define ADDR_NODE_ELMS ((U64CACHE_ALIGN - sizeof(U64Stack) - sizeof(RBIndex) - sizeof(u64)) / sizeof(U64Frame))

#define SIZE_NODE_SIZE  (CACHELINE * 4)
#define SIZE_NODE_ELMS  (SIZE_NODE_SIZE - sizeof(ListIndex) - sizeof(U64Stack)) / sizeof(u64)

#define SIZE_POWERS     5
#define SIZE_CONV_LISTS ( SIZE_POWERS * (SIZE_POWERS + 1) / 2 )
#define SIZE_HUGE_LIST  SIZE_CONV_LISTS
#define SIZE_MAX_BUCKET (1 << ((SIZE_POWERS << 1)))
#define SIZE_LISTS      (SIZE_CONV_LISTS + 1)

typedef u64 U64Frame;

typedef struct {
    U64Stack  Cache;
    U64Frame  Frames[SIZE_NODE_ELMS];
    ListIndex Index; 
} SizeNode;

typedef struct {
    u64*      OwnerPtr;
    U64Frame* EntryPtr; 
    SizeNode* Node;
} Frame;

struct Frames {
    RBTree AddrTree;
    RWLock Lock;
    USLAB  Slab;
    List   Lists[SIZE_LISTS];
    usize  Bytes;
    usize  TotalBytes;
};

typedef struct {
    U64Stack Cache;
    u64 Buffer[ADDR_NODE_ELMS];
    u64 Key;
    RBIndex Index;
} AddrNode;