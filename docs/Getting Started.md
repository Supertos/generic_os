# Getting Started

## Folder Structure
### Root
GenericOS repository contains 4 folders:
- kernel: contains kernel itself, its code, makefile and other resources
- loader: contains loader code, built with posix-uefi
- shared: shared defines and functions used by both kernel and loader
- uefi: posix-uefi from bzt

### Kernel
- kernel/Domain: contains Domain and Core implementation
- PhysicalMemory: contains Caches and Frames implementation
- Primitives: contains various structures, such as RWLock, RBTree, etc
- VirtualMemory: contains VTree, VSpace, VRegion, VAllocator implementation
- Domains.h: Public Domain API
- MemoryManager.h: Public Memory Manager API

## Build Process
- `shared/*` files assembled into object files in `build/` directory
- `kernel/Makefile` assembles object files from sources in `shared/` and `kernel/` to `build/` directory
- `kernel/Makefile` builds `build/kernel.elf` from object files in `build/` directory
- `uefi/Makefile` assembles object files from source in `shared/` and `loader/` to `build/` directory with `Microsoft x64 ABI`
- `uefi/Makefile` builds temporary output file from object files in `build/` directory
- `uefi/Makefile` objcopy'es temporary output file to `build/loader.efi`
- `build/loader.efi` is copied into `::/EFI/BOOT/` of `build/generic-os.img` image
- `build/kernel.elf` is copied into `::/ELF/CORE/` of `build/generic-os.img` image

## Code Style guidelines
- Never use `!ptr`, use `ptr != NULL`: It is invalid in GenericOS codebase and will result in unexpected behavior since NULL is not 0.

## Base.h and basic defines
GenericOS uses custom typedefs for types and __custom NULL value__. Base.h defines following:

### [NULL](../shared/Base.h#:~:text=%23define%20NULL)
NULL essentially means invalid address. Since 0x0 is perfectly valid on x86-64, default NULL would cause problems with Physical Memory Manager. It was decided that NULL will be `(void*)(~0ULL)` `(or 0b1111...111)` inside GenericOS kernel.
```c
#define NULL ((void*)(~0ULL))
```

However, since UEFI relies on proper C NULL value, Generic Bootloader uses default 
```c
#define NULL (void*)0
```

This behavior is controlled by `GENERIC_LOADER` define. When `GENERIC_LOADER` is undefined, Base.h redefines `NULL` to `((void*)(~0ULL))`, otherwise it does nothing.

### [Types](../shared/Base.h#:~:text=typedef%20unsigned%20char)
GenericOS numeric types are:
| Standard C name | GenericOS Type | Note |
|-|-|-|
| `unsigned char` | `u8` | |
| `unsigned short` | `u16` ||
| `unsigned int` | `u32` ||
| `unsigned long long` | `u64` ||
| `unsigned long long __attribute__((mode(pointer)))` | `usize` ||
| `signed int` | `i32` | Used in `.ELF` parsing |
| `signed long long` | `i64` | Used in `.ELF` parsing |
| same as u16 | `CoreID` | APIC ID of a single Core |
| same as u16 | `ContextID` | PCID (x86) or ASID (ARM) |
| same as u8 | `DomainID` | ID of ProximityDomain (NUMA Node) |

### Convenience Defines
| Define | Description |
|-|-|
|`bitsof(type)`| Amount of bits needed to represent `type`, e.g. 64 for `bitsof(u64)`|
|`AlignUp(base, align)`| `base` aligned up to `align` |
|`AlignDown(base, align)`| `base` aligned down to `align` |
|`Aligned(base, align)`| `true`, if `base` is aligned to `align`, otherwise `false` |
|`MASK(type, pos, length)`| value of `type` with [pos, pos + length] bits set to 1, others set to 0. Handles length > bitsof(type) |

### [Memory](../shared/Base.h#:~:text=Memory%3B)
Memory object describes a memory region.
```c
typedef struct { 
    u64 Base; // Base address of memory region 
    u64 End; // First address after the Base that does not belong to memory region
} Memory;
```

#### Constants
- `MemoryInvalid`: Invalid Memory Region, both `Base` and `End` set to `NULL`

#### Methods
##### MemoryNew
```c
MemoryNew(base, size);
```
Returns new Memory starting from base, ending with size.

##### MemoryIntersect
```c
static inline Memory MemoryIntersect( Memory a, Memory b );
```
Returns intersection of `a` and `b` or `MemoryInvalid` if a does not intersect with b.

##### MemorySuits
```c
static inline bool MemorySuits( Memory self, usize size, usize align );
```
Returns `true` if `self` contains `(Memory){ AlignUp(self.Base, align), AlignUp(self.Base, align) + size }`. behavior is undefined, if `align` is 0.

##### MemoryDrain
```c
static inline Memory MemoryDrain( Memory* self, usize size );
```
Subtracts from `self->Base` `size` and returns subtracted `Memory`.

##### MemoryEqual
```c
static inline bool MemoryEqual( Memory a, Memory b );
```
Returns `true` if `a` describes the same memory as `b`.

##### MemoryContains
```c
static inline bool MemoryContains( Memory self, usize ptr );
```
Returns `ptr` is inside `self` bounds.

##### MemoryValid
```c
static inline bool MemoryValid( Memory self );
```
Returns `true` if `self` is valid. That is, if size of memory described by `self` is greater than 0, and `self.Base` is less than `self.End`.

##### MemorySize
```c
static inline usize MemorySize( Memory self )
```
Returns size of memory region described by `self` if `self` is valid. Otherwise, returns 0.

##### MemorySet
```c
static inline char* MemorySet( Memory self, u8 val );
```
Fills memory region, described by `self` with `val`

### [Allocator](../shared/Base.h#:~:text=Allocator%3B)
```c
typedef Memory (*Allocate)( void* self, usize size, u64 align );
typedef void (*Free)( void* self, Memory free );

typedef struct { Allocate From; Free To; void* Self; } Allocator;
```

Allocator describes a source of memory (be it physical or virtual), and acts as wrapper for indifferent consumer.

#### Constants
- `AllocatorInvalid`: `Allocator` object with all its field set to `NULL`

#### Methods

##### From
```c
static inline Memory From( Allocator from, usize size, usize align );
```
Calls `from.From` and returns its result.

##### To
```c
static inline void To( Allocator to, Memory mem );
```
Calls `to.To`.

##### Replenish
```c
static inline Memory Replenish( Allocator from, Allocator to, usize size, usize align );
```
Moves memory from `from` to `to` until last moved fragment passes `MemorySuits(mem, size, align)` check.

##### AllocatorNew
```c
AllocatorNew(from, to, self)
```
Returns `Allocator` object with fields set to arguments respectively. Used for convenience - allows any type of pointer to be passed there.

### Helpers 
```c
static inline u64 CAS64( u64* trg, u64 val );
static inline u32 CAS32( u32* trg, u32 val );
static inline u16 CAS16( u16* trg, u16 val );
```
Sets `*trg` to `val` atomically and returns old value of the `*trg`

```c
static inline void ValueCAS64( u64* trg, u64 expect, u64 val );
static inline void ValueCAS32( u32* trg, u32 expect, u32 val);
static inline void ValueCAS16( u16* trg, u16 expect, u64 val);
```
Sets `*trg` to `val` atomically, only if `*trg` is set to `expect`.

| Function | Description |
|-|-|
|`u64 CAS64( u64* trg, u64 val )`| Sets `*trg` to `val` atomically and returns old value of the `*trg`. Versions for u32 and u16 exist as well |
|`void ValueCAS64( u64* trg, u64 expect, u64 val )`| Sets `*trg` to `val` atomically, only if `*trg` is set to `expect`. Versions for u32 and u16 exist as well |

| Define | Description |
|-|-|
| `tail` | [[gnu::musttail]] |
| `may_alias` | `__attribute__((__may_alias__))` |
| `packed` | `__attribute__((__packed__))` |
| `unused` | `__attribute__((unused))` |
| `compiler_barrier` | Forbids compiler from reordering operations around it |

### Architecture-Specific definitions
- `PAGE`: size of hardware pages in bytes
- `HUGE_PAGES`: size of hardware hugepage in pages
- `GIGA_PAGES`: size of hardware gigapage in pages
- `HUGEPAGE`: size of hardware hugepage in bytes
- `GIGAPAGE`: size of hardware gigapage in bytes
- `BITS`: size of byte in bits
- `CACHELINE`: size of hardware cacheline in bytes
- `PAGE_BITS`: minimum amount of bits required to represent `PAGE`
- `HUGE_BITS`: minimum amount of bits required to represent `HUGEPAGE`
- `VIRT_BITS`: maximum bits supported when addressing virtual space
- `PHYS_BITS`: maximum bits supported when addressing physical space
- `PCID_BITS`: bit count of Process Context ID
- `architecture_barrier`: barrier for weak memory order architecture, no-op on x86

