/* Supertos Industries (2012 - 2025)
 * SLUB Kernel Allocator with residual blocks.
 * Single CPU owns SLUB and allocates objects from it's double-linked lists. No slab shall ever exceed 2K (But it's very wasteful, so we stick to using only 512)
 * Other CPUs' are able to remote free objects. Concurrency guaranteed with atomic (LOCK prefix) operations.
 * SLAB consists of three parts: Metadata, Slot Bitmap/Residual Blocks, Main Blocks
 * By design, block size is tightly linked to it's offset. Every block offset is no less than it's size. No need in tracking block sizes in places other than SLAB metadata.
 * For residual blocks this plays important role - it guarantees that memory [End of metadata, begin of first block] is fully utilised.
 * Residual block headers are overwritable by user and shall not contain any important information.
 * Full pages are not tracked.
 */

#include "utils/Architecture.h"
#include "utils/Atomic.h"
#include "utils/Spinlock.h"
#include "MemoryManager.h"
#include "NUMA.h"


#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define SLABS_MIN_ALLOC (size_t)(16)
#define SLABS_MIN_ORDER BSR(SLABS_MIN_ALLOC)

#define SLABS_TOT_ORDERS (size_t)(6) // Max allocation of size 512.

#define SLABS_SLOTS_PER_CACHELINE (CACHELINE_LENGTH / SLABS_MIN_ALLOC)
#define SLABS_BITMAP_SLOT_COUNT (PAGE_SIZE / SLABS_MIN_ALLOC) // Elements in bitmap.
#define SLABS_BITMAP_COUNT (SLABS_BITMAP_SLOT_COUNT / (sizeof(bitmap_t) * 8))
#define SLABS_BITMAP_LENGTH ((BSR(SLABS_BITMAP_COUNT - 1) + 1) * CACHELINE_LENGTH) // Bitmap length aligned to CACHELINE_LENGTH.

#define MASK_PAGE_OFFSET (PAGE_SIZE - 1)
#define MASK_PAGE_BEGIN ~(PAGE_SIZE - 1)

#define min(a, b) ({ __typeof__(a) _a = (a); __typeof__(b) _b = (b); __typeof__(a) _result = _a > _b ? _b : _a; _result; })
#define max(a, b) ({ __typeof__(a) _a = (a); __typeof__(b) _b = (b); __typeof__(a) _result = _a < _b ? _b : _a; _result; })

typedef uint64_t bitmap_t; // Just to make it portable.

typedef struct SLUB SLUB;
typedef struct SLAB SLAB;
struct SLAB {
    void* Next;
    void* Prev;
    SLUB* Owner;

    uint16_t ElementLength;
    uint16_t ResidualElementCount;
    uint16_t MainElementCount;

    spinlock_t InUse;
    
    uint8_t _Reserved; // TODO: This is too reliant that spinlock_t is uint8_t.

    bitmap_t Bitmap[SLABS_BITMAP_COUNT];
} __attribute__((aligned(CACHELINE_LENGTH)));


typedef struct ResidualSLAB ResidualSLAB;
struct ResidualSLAB {
    void* Next;
    void* Prev;
};

struct SLUB {
    void* Partial[SLABS_TOT_ORDERS];
    spinlock_t RemoveLock[SLABS_TOT_ORDERS];
    NUMAArena* Arena;
};

static inline SLAB* SLABObject( void* object ) { 
    return (SLAB*)( (uintptr_t)object & MASK_PAGE_BEGIN ); 
}

static inline uintptr_t SLABObjectOffset( void* object ) { 
    return (uintptr_t)object & MASK_PAGE_OFFSET; 
}

static inline bool SLABResidual( void* object ) {
    return ((uintptr_t)object & MASK_PAGE_OFFSET) != 0;
}

static inline size_t SLABResidualTotal( SLAB* slab ) {
    if( slab->ElementLength < CACHELINE_LENGTH ) return 0;
    return BSF( slab->ElementLength ) - BSF( CACHELINE_LENGTH );
}

static inline uintptr_t SLABMainOffset( SLAB* slab ) {
    return min( sizeof(SLAB), slab->ElementLength );
}

static inline size_t SLABMainTotal( SLAB* slab ) {
    return (PAGE_SIZE - SLABMainOffset( slab )) / slab->ElementLength;
}

static inline size_t SLABCanFree( SLAB* slab ) {
    return SLABResidualTotal( slab ) == slab->ResidualElementCount && SLABMainTotal( slab ) == slab->MainElementCount;
}

static inline size_t SLABSizeToOrder( size_t size ) {
    return BSR(size - 1) + 1 - BSR(SLABS_MIN_ALLOC);
}

void* SLABFirst( SLAB* slab ) {
    size_t slot, base;
    for( base = 0; base < SLABS_BITMAP_COUNT; ++base )
        if( (slot=BSF(slab->Bitmap[base])) != ~0ULL ) break;
    
    if( slot == ~0ULL ) return NULL;

    uintptr_t out = (uintptr_t)slab + (slot + (sizeof(bitmap_t) * 8) * base) * SLABS_MIN_ALLOC;
    return (void*)out;
}


void SLABSet( void* object, bool isFree ) {
    SLAB* slab = SLABObject( object );

    uintptr_t offset = SLABObjectOffset( object );
    size_t size = min( offset, slab->ElementLength ); // Residual block handling (First block of size n is always at min(n, sizeof(SLAB))
    size_t slots = size / SLABS_MIN_ALLOC;

    size_t slot = (offset / SLABS_MIN_ALLOC) % (sizeof(bitmap_t) * 8);
    size_t base = (offset / SLABS_MIN_ALLOC) / (sizeof(bitmap_t) * 8);

    size_t mask = ((1ULL << slots) - 1) << slot;

    if( isFree ) AtomicOr64( &slab->Bitmap[base], mask );
    else AtomicAnd64( &slab->Bitmap[base], ~mask );
}

// So the first element can only be removed by owner.
// What if this is big element with residual elements?
// We can atomically spinlock every residual block with page lock.
// Alright, so other threads can remove elements with id > 0.
// Is it benefecial? Let's see: we try to remove element with ID 0 on free.
// We need to update head. But allocation itself is not atomic: we lookup first free element and atomically mark it as occupied.
// Then if we remove element with ID > 0 we can guarantee that owner won't be allocating there since it allocates on ID 0.
// But is it correct? Let's assume we removing not a single element, but the whole page with it's residual elements.
// What if some of it's blocks have ID == 0 in their respective lists. Then we have to lock whole page.
// Then owner is locked from allocation. It's bad. What if owner moves onto next element then?
// But what if another thread also removes it? We can't guarantee anything. Or can we?
// We have only one allocating thread (owner) and others remote freeing. This means we remove either one element (when allocating) or whole single page (when freeing).
// This means we can't possibly encounter a situation where owner tries to allocate part of to-be removed whole page since it atomically marked as one.
// So, the thing is: we don't have to allow first element removal only to owner.
// All we have to do is lock element removal (to avoid breaking Prev and Next members), atomically set to-be removed flag and atomically update prev's next.
// But wait: it's two actions. We can't ensure them being atomic.
// Let's review problem: we have one allocating core that and some removing cores.
// Allocating core is known to jump over to-be removed pages.
// But what if it jumps onto to-be removed pages after it being freed?
// Example: removing page -> removing page -> removed page -> free page.
// In this example allocator would jump to removed with possibly corrupted data. This is bad.
// We can always atomically change Next first to prevent allocating core from jumping to to-be removed page.
// Let's test:
// We update page ID 1 to-be removed next. Now it points to ID 2.
// Allocator jumps to page ID 2 and begins allocation.
// Other thread updates page ID 2 to-be removed next.
// This is bullshit. How can we prevent that?
// We can atomically update element counter before allocation.
// We could update page ID 2 FreeElementCount to stop removing but is this really an option?
// We need to: 1. Check to-be removed flag, decrease FreeElementCount. This is not atomically made.
// But wait, if we update Next member first, we don't have to check to-be removed flag since allocating core jumps over these ones.
// But wait again. Imagine chain freeing:
// ID 1 Free -> ID 0 Next Updated
// ID 2 Free -> ID 1 Next Updated
// In this case we have ID 0 point to ID 2 but it's to-be freed.
// But wait: we have list remove lock. This means we can't possibly update ID 2 before ID 2's Prev is updated.
// But what about head element? What happens when we remove ID 0?
// We atomically update head. We don't even have to set Prev's and Next's.
// Alright. This seems correct. Let's review:
// We have global remove lock meaning only one thread can remove element from the list at a time.
// First thing we do, is changing Next element of Prev atomically, so we ensure the only allocating core won't ever find to-be removed page.
// Then we set global remove lock. WAIT. What if allocator core is going to allocate other part of out page? What about it's resodial parts?
// Shit. This is not atomic.

// Alright, let's think again.
// Since just removing element from search is not an option, we can return to setting it's flag.
// Since element removal is performed only by single thread at a time, there can be no such thing as double set-to-remove pages.
// This means we can't really jump-over to already freed pages.
// Or can we? Let's test.
// Free'er sets flag.
// Allocator jumps to element with flag.
// Free'er frees page.
// Allocator jumps to garbage and dies.
// This is very bad.
// If only one element is being removed at a time, why not just have pointer to second element?
// If allocator sees the flag it can jump to second page that is guaranteed to not be removed at the moment. Then atomically decrease FreeElementCount.
// Wait. How can we be sure this works? Let's test:
// Free'er sets the flag.
// Allocator tests the flag after it's set but before head pointer's changed and page's removed.
// Free'er updates pointer. Now second pointer is, for example, is empty.
// Allocator jumps to second page in the list via special pointer and overwrites UEFI.
// This is bad. Since we've establlished we have to let other threads to remove first element we have to do something about it.
// What about having universal page-in-use state?
// Instead of having to-be removed flag, we use in-use flag.
// When free'er removes first page it sets flag.
// Wait. Why not just atomically update head?
// We can't do that: we have to update flag first.
// Alright, so we set the flag. Then set the head.
// Then why not just wait for removal as allocator?
// It's slow and exhausting.
// It's not slow - atomically updating flag and Next is not really slow.
// But we'd __asm__ ("pause" :::) there. It's ~30-100 cycles.
// We can implement another function without pauses.
// Alright. Let's review:
// Free'er sets first page in-use
// Allocator waits for head to have it's in-use flag removed.
// Free'er updates head and then frees the page.
// Allocator gets new first page and sets it to be in-use.
// Another free'er sees that first page is in use and aborts.
// Allocator allocates.
// Happy end.

// Now let's review the page freeing.
// Having only one thread to remove a page seems like bullshit.
// This means if all threads are removing page we're getting huge latency for the least-lucky core.
// Alright, let's try to allow any core to free. What problems exactly appear:
// Not atomic pointer update: Next and Prev are getting updated not simultaniously.
// How long it takes to update two pointers? ~4-20 * ~(100 - 300) cycles.
// Alright, what if we could just abort freeing when it's locked?
// Then we have to make someone else remove it.
// Let it be allocator.
// This seems bad: we can't just allow it to happen - what if we have thousands free pages?
// We either make allocator to free them all at once or they become deadweight somewhere in list's end.
// Alright, what if we lock not all removals, but only one page.
// Then we have to check if element->Next->Next and element->Prev->Prev have no in-use flag.
// But this raises problem - allocator sets in-use flag without actually modifying the list.
// What if we make bitmap? Let it be two flags: in-use : 4; allocating: 4; Then we just need to test if it's 0b1111.
// Alright. How much does it takes to test? 
// ~4-20 * ~(100 - 300) cycles + branching (what if element->Next does not exist? etc). This seems more laggy than whole-list spinlock.
// Since we could also remove __asm__ ("pause" :::) here, it would cost us the same ~4-20 cycles. But what about bus?
// In neighbor testing we commence 5 bus reads with 5 possible cache-misses.
// In list spin-lock we commence ~20 bus reads with 1 possible cache-misses.
// Since spinlock is volatile we have great false-sharing here.
// But wait. Reading spinlock (if it's not volatile) means we have to read bus only once to get it in the cache.
// When spinlock gets updated it's automatically gets false-shared onto us! So we commence only 1 bus read with 1 possible cache-miss.
// Wait. Does xchg does false-sharing only when value changes? No.
// We could use cmpxchg. This way we can use spinlock that is not volatile and be a happy boi.

// Alright, we've established that false-sharing is our friend in spinlock-driven system.
// But now we have to somehow eradicate false-sharing on free.
// Let's review the problem: allocator and free use the same cacheline.
// This means we get false-sharing. That is, like, crazy: 30-200 cycles on every core freeing.
// What if we store local copy of masks to be applies in separate cachelines?
// This baloons metadata and we have to somehow iterate and store them for every core.
// This is unacceptable.
// What if we could store linked list of free'd elements: element addr and then apply mask?
// This is bullshit: we morph slab into linked list and enslaving owner to do all returning routine before actually allocating anything.
// What if we separated odd and even elements into two masks and update it. This means we could get twice less false-sharings on free.
// Grok, is it true? Let's see:
// 6 cores remote free. Let's pretend we live in perfect world and we get triplet cores writing to free-masks.
// This means we get 3 false-sharings on each core.
// Then we need to apply changes- WAIT.
// Applying changes means that owner has to walk through all pages (full ones too) and not only false-share all these cache-lines, but also apply it to the real one bitmap.
// This is utter bullshit.
// But wait. We care little for relevance of non-head pages since we don't allocate from them. This means we won't walkthrough.
// This is nice, but it still needs saving in separate cachelines for each core for it to have any benefit. (This way we have 0 false-sharings on free and 1 false-sharing on each core on applying changes)
// This means we have to baloon metadata in order to minimize false-sharings.
// Let's test: we have 64 cachelines in a 4K page.
// This means we can limit ourselves to 63 cores per page when page becomes basically useless.
// This is another utter bullshit.
// Alright. How much  remote frees is there anyways?
// Giving cores separate cachelines to write to is not an option. Or is it?
// Let's give every page 'To be freed' linked list.
// Whenever we try to push element to-free, we just push it to second (first if there's no second) element.
// Why second? Because updating second element means we're not updating list's head means we're not touching. WAIT. We're reading top anyways. It means we can update it. Right?
// No. False-sharing happens only on writes. Means we should minimize writings to head.
// So, let's review:
// Element gets casted to 'Element to free' with first member being pointer to the next one.
// Wait. This creates another race condition. Even if we atomically update second element we can't be sure owner is not reading all this shit we've dumped.
// This also makes structure bigger than 64 bytes. We could place it onto next cacheline and lose a little bit of space.
// Wait. When applying changes, owner core loads ton of cachelines. (Up to 63 misses).
// This is bad. Loading from bus is longer than false-sharing anyways.
// Let's test if it's any good:
// Default (Commit to main) approach:
// 63 frees - 63 false sharings possibly to every core including owner.
// Free-list thingy approach:
// 63 frees - Possibly much more false-sharings on small objects (< 64) but only on some cores. Risky!
// Plus 64 cache-misses when applying (including metadata) which means bus loading which is slow, but no false-sharings since it's only reading.
// Then finally applying to the metadata and only afterwards proceeding to allocate data for owner.

// Freelist approach seems better on systems with lots of cores, big (>=64) objects with additional allocation latency.
// However it not only baloons code, but creates 64 cache-misses as well. However, dynamically allocated objects are usually used often in OS, meaning
// cache would pretty often have these, which means less of cache-misses. Yet modern OSes tend to have a lot of memory hence making it unlikely that objects are actually in the cacheline.
// We could exchange cache-misses for false-sharing with having not just freed elements in the list, but have them store actual update masks if possible. However, this complicates system even futher - we have to somehow decide whether push new element to freelist or update existing one with new mask.


static inline void** SLABNext( void* obj ) { return SLABResidual(obj) ? &((ResidualSLAB*)obj)->Next : &((SLAB*)obj)->Next; }
static inline void** SLABPrev( void* obj ) { return SLABResidual(obj) ? &((ResidualSLAB*)obj)->Prev : &((SLAB*)obj)->Prev; }

static inline bool SLABCanRemoveObjectFromList( void* obj ) {
    if( SLABResidual(obj) ) return true; // Residual object always have one slot hence cannot be taken and freed simultaniously.
    SLAB* slab = SLABObject( obj );
    return SLABMainTotal( slab ) == slab->MainElementCount;
}

void SLABRemoveFromList( SLUB* slub, void* obj ) {
    SLAB* slab = SLABObject( obj );
    size_t order = SLABSizeToOrder( slab->ElementLength );

    void** head = &slub->Partial[order];
    spinlock_t* removeLock = &slub->RemoveLock[order];

    SpinlockLock( &slab->InUse );
    if( SLABCanRemoveObjectFromList(obj) ) {
        SpinlockLock( removeLock );

        void* next = *SLABNext( obj );
        void* prev = *SLABPrev( obj );

        if( obj == head ) {
            if( next ) *SLABPrev( next ) = NULL;
            *head = next;
        }else{
            if( next ) *SLABPrev( next ) = prev;
            *SLABNext( prev ) = next;
        }
        SpinlockRelease( removeLock );
    }
    SpinlockRelease( &slab->InUse );
}


void SLABAddToList( void** head, void* obj, spinlock_t* removeLock ) {
    *SLABPrev( obj ) = NULL;
    SpinlockLock( removeLock ); // To ensure our previous head never modified mid-insertion
    *SLABNext( obj ) = AtomicExchange64( (uint64_t*)head, (uint64_t)obj );

    void* next = SLABNext( obj );
    if( next ) *SLABPrev( next ) = obj;

    SpinlockRelease( removeLock );
}


void SLABDeletePage( SLUB* slub, void* randomPointer ) {
    SLAB* slab = SLABObject( randomPointer );

    SpinlockLock( &slab->InUse );
    if( SLABCanFree(slab) ) {
        SLABRemoveFromList( slab );

    }
    SpinlockRelease( &slab->InUse );

}