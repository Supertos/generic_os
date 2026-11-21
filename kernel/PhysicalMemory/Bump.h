/* Supertos Industries (2012 - 2025)
 * Bump Allocator used in early stages.
 * Used to place heterogenous data inside big memory region.
 * Always aligned to usize.
 * Crashes kernel exhausted.
 */
#pragma once

#include "Base.h"
#include "Primitives/RWLock.h"

typedef struct {
    char* Mark;
    usize Base;
    usize End;
    RWLock Lock;
} Bump;

static inline bool CanFromBump( Bump* bump, usize size ) {
    return bump->Base + size <= bump->End;
}

static inline Memory FromBump( Bump* bump, usize size, usize align ) {
    ACQUIRE_AUTO(&bump->Lock);
    align = align ?: 1;
    usize out = bump->Base;
    
    bump->Base = AlignUp(out, align);
    if( !CanFromBump(bump, size) ) KernelCrashUndBurn(bump->Mark);
    bump->Base = AlignUp( out + size, sizeof(usize) );
    return MemoryNew(out, size);
}

static inline Bump BumpNew( usize begin, usize size, char* mark ) {
    return (Bump){ 
        .Base = AlignUp(begin, sizeof(usize)),
        .End = begin + size, 
        .Mark = mark
    };
}
