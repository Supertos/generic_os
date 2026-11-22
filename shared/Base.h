/* Supertos Industries (2012 - 2026) 
 * Base definitions and types.
 */ 
#pragma once

#ifndef GENERIC_LOADER
    #define NULL ((void*)(~0ULL))
    #define min(x, y)                    ({ typeof(x) _x = x; typeof(y) _y = y; _x < _y ? _x : _y; })
    #define max(x, y)                    ({ typeof(x) _x = x; typeof(y) _y = y; _x > _y ? _x : _y; })
#else
    #include <uefi.h>
#endif

#define tail                         [[gnu::musttail]]

#define offsetof(type, member)       (usize)&(((type*)0)->member)
#define bitsof(type)                 (sizeof(type) * BITS)
#define countof(arr)                 (sizeof(arr) / sizeof(arr[0]))
#define idof(arr, mem)               (((usize)mem - (usize)arr) / sizeof(typeof(arr[0])))

#define may_alias                    __attribute__((__may_alias__))
#define packed                       __attribute__((__packed__))
#define unused                       __attribute__((unused))
#define exported                     __attribute__((visibility("default")))

#define AlignDown(base, align)       ({ u64 __base = (u64)(base); u64 __align = (u64)(align); (typeof(base))(__base & ~(__align - 1)); })
#define AlignUp(base, align)         ({ u64 _base = (u64)(base); u64 _align = (u64)(align); (typeof(base))AlignDown(_base + (_align - 1), _align); })
#define Aligned(base, align)         ({ u64 _base = (u64)(base); u64 _align = (u64)(align); !(_base & (_align - 1)); })

#define MemoryNew(base, size)        (Memory){ (usize)(base), (usize)base + (usize)(size) }
#define AllocatorNew(from, to, self) (Allocator){ (Allocate)from, (Free)to, (void*)self }
#define AllocatorInvalid             (Allocator){ NULL, NULL, NULL }
#define compiler_barrier             __asm__ __volatile__ ("nop;" : : : "memory")

#define MemoryInvalid                (Memory){(u64)NULL, (u64)NULL}

#define LOG2_32(x) ( \
    ((x) & 0xFFFF0000 ? 16 : 0) + \
    (((x) & 0xFF00FF00 ? 8 : 0)) + \
    (((x) & 0xF0F0F0F0 ? 4 : 0)) + \
    (((x) & 0xCCCCCCCC ? 2 : 0)) + \
    (((x) & 0xAAAAAAAA ? 1 : 0)) \
)

#define MASK(type, pos, length) ({ 							\
	usize _length = length, _pos = pos;					    \
	type _0 = 0, _1 = 1;									\
															\
	bool _ubShift = _length >= sizeof(type) * BITS;	        \
	type _mask = _ubShift ? ~_0 : ((_1 << _length) - _1);	\
	_mask << _pos; 											\
})

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef signed int         i32;
typedef signed long long   i64;

typedef unsigned long long __attribute__((mode(pointer))) usize;

typedef struct { u64 Base; u64 End; } Memory;

typedef Memory (*Allocate)( void* self, usize size, u64 align );
typedef void (*Free)( void* self, Memory free );
typedef bool (*Predicat)( void* self, void* param );

typedef struct { Allocate From; Free To; void* Self; } Allocator;

typedef u16 CoreID;
typedef u16 ContextID;
typedef u8  DomainID;

typedef struct ExternalSymbol ExternalSymbol;

typedef struct PageWalker PageWalker;

static inline Memory MemoryIntersect( Memory a, Memory b ) {
    if( a.End < b.Base || a.Base > b.End ) return MemoryInvalid;
    return (Memory){ max(a.Base, b.Base), min(a.End, b.End) }; 
}

static inline bool MemorySuits( Memory self, usize size, usize align ) {
    return AlignUp(self.Base, align) + size <= self.End;
}

static inline Memory MemoryDrain( Memory* self, usize size ) {
    self->End -= size;
    return MemoryNew(self->End, size);
}

static inline bool MemoryEqual( Memory a, Memory b ) {
    return a.Base == b.Base && b.End == a.End;
}

static inline bool MemoryContains( Memory self, usize ptr ) {
    return ptr >= self.Base && ptr <= self.End;
}

static inline bool MemoryValid( Memory self ) {
    return self.End > self.Base;
}

static inline usize MemorySize( Memory self ) {
    return MemoryValid(self) ? self.End - self.Base : 0;
}

static inline Memory Replenish( Allocator from, Allocator to, usize size, usize align ) {
	Memory out;
    do {
        out = from.From(from.Self, size, align);
        if( !MemoryValid(out) ) break;
        to.To(to.Self, out);
    } while( !MemorySuits(out, size, align) );

    return out;
}

static inline Memory From( Allocator from, usize size, usize align ) {
    return (from.From == NULL) ? MemoryInvalid : from.From(from.Self, size, align);
}

static inline void To( Allocator to, Memory mem ) {
    if( to.To && to.Self ) to.To(to.Self, mem);
}

static inline char* MemorySet( Memory self, u8 val ) {
    for( usize i = 0; i < MemorySize(self); i++ )
        ((u8*)self.Base)[i] = val;
    return (char*)self.Base;
}

static inline void MemoryCopy( Memory from, Memory to ) {
    for( usize i = 0; i < min(MemorySize(from), MemorySize(to)); i++ )
        ((char*)to.Base)[i] = ((char*)from.Base)[i];
}

static inline bool MemoryCompare( Memory a, Memory b ) {
    if( MemorySize(a) != MemorySize(b) ) return false;
    for( usize i = 0; i < MemorySize(a); i++ )
        if( ((char*)a.Base)[i] != ((char*)b.Base)[i] ) return false;

    return true;
}

static inline bool StringCompare( char* a, char* b ) {
    for( usize i = 0;  a[i] || b[i]; i++ )
        if( a[i] != b[i] ) return false;

    return true;
}

static inline usize StringCopy( char* from, char* to ) {
    usize i = 0;
    do {
        to[i] = from[i];
    } while( from[i++] );

    return i;
}

#define CAS_HELPER(size)                                                        \
    static inline u64 CAS##size( u##size* trg, u##size val ) {                  \
        u##size expected = *trg;                                                \
        while( !TryCAS##size(trg, &expected, val) );                            \
        return expected;                                                        \
    }                                                                           \
                                                                                \
    static inline void ValueCAS##size( u##size* trg, u##size expect, u##size val ) {  \
        u##size a = expect;                                                     \
        while( !TryCAS##size(trg, &a, val) ) a = expect;                        \
    }

#if defined(__x86_64__)
    #define PAGE       4096ULL
    #define HUGE_PAGES 512ULL
    #define GIGA_PAGES (512ULL * 512ULL)
    #define HUGEPAGE   (PAGE * HUGE_PAGES)
    #define GIGAPAGE   (PAGE * GIGA_PAGES)
    #define BITS       8
    #define CACHELINE  64

    #define PAGE_BITS  12
    #define HUGE_BITS  21
	
	#define VIRT_BITS  57
	#define PHYS_BITS  52
	#define PCID_BITS  12
    
	#define architecture_barrier ;

    #include "x86-64/Asm.h"
    #include "x86-64/PageWalker.h"
    #include "ACPI/MachineInfo.h"
#else
    #error "Unsupported machine."
#endif

CAS_HELPER(64)
CAS_HELPER(32)
CAS_HELPER(16)

#include "Debug.h"