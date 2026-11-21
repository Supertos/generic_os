/* Supertos Industries (2012 - 2026) */
#pragma once

#include "Base.h"

#define __ASM8 b
#define __ASM16 w
#define __ASM32 l
#define __ASM64 q
#define __ASMsize q

#define __ASM_SIZE(size) __ASM##size
#define __ASM_STRING_INT(x) #x
#define __ASM_STRING(x) __ASM_STRING_INT(x)

#define __ASM(mnem, size, ops) mnem __ASM_STRING( __ASM_SIZE(size) ) " " ops ";"

#define DECL_FOR_SIZE( size ) 														\
static inline usize TZCNT##size( u##size in ) { 									\
	bool fail = false; 																\
	u##size out = 0; 																\
	__asm__ __volatile__ ( 															\
		__ASM("tzcnt", size, "%2, %1")  											\
		: "=@ccc"(fail), "=r"(out) 													\
        : "r"(in) 																	\
        : 																			\
	); 																				\
	return fail ? ~0ULL : out; 														\
} 																					\
																					\
static inline usize LZCNT##size( u##size in ) {										\
    bool fail = false;																\
    u##size out = 0;																\
    __asm__ __volatile__ (															\
		__ASM("lzcnt", size, "%2, %1")												\
        : "=@ccc"(fail), "=r"(out)													\
        : "r"(in)																	\
        :																			\
    );																				\
    return fail ? ~0ULL : out;														\
}																					\
																					\
static inline void LockOr##size( u##size* in, u##size orV ) {						\
	__asm__ __volatile__ (															\
		__ASM("lock or", size, "%1, %0")											\
        : "+m"(*in)																	\
        : "r"(orV)																	\
        : "memory", "cc"															\
    );																				\
}																					\
																					\
static inline void LockAnd##size( u##size* in, u##size orV ) {						\
	__asm__ __volatile__ (															\
		__ASM("lock and", size, "%1, %0")											\
        : "+m"(*in)																	\
        : "r"(orV)																	\
        : "memory", "cc"															\
    );																				\
}																					\
																					\
static inline bool TryCAS##size( volatile u##size* in, volatile u##size* old, u##size newVal ) {	\
    bool success = false;															                \
    __asm__ __volatile__ (															                \
		__ASM("lock cmpxchg", size, "%3, %1")										                \
        : "=@ccz"(success), "+m"(*in), "+a"(*old)									                \
        : "r"(newVal)																                \
        : "memory"																	                \
    );																				                \
    return success;																	                \
}																					                \
																					                \
static inline u##size FetchAdd##size( volatile u##size* in, u##size add ) {						    \
    __asm__ __volatile__ (															                \
		__ASM("lock xadd", size, "%1, %0")											                \
        : "+m"(*in), "+a"(add)														                \
        :																			                \
        : "cc", "memory"															                \
    );																				                \
    return add;																		                \
}               																					\
																					                \
static inline u##size FetchSub##size( volatile u##size* in, u##size sub ) {						    \
    return FetchAdd##size( in, -sub );												                \
}

DECL_FOR_SIZE(16);
DECL_FOR_SIZE(32);
DECL_FOR_SIZE(64);

typedef struct { 
	u32 eax; 
	u32 ebx; 
	u32 ecx; 
	u32 edx; 
} CPUIDResult;

static inline void Pause() {
    __asm__ __volatile__ ("pause;");
}


static inline void UD2() {
    __asm__ __volatile__ ("ud2;");
}

static inline CPUIDResult CPUID( u32 eax, u32 ecx ) {
    CPUIDResult out = { .eax = eax, .ecx = ecx };

    __asm__ __volatile__ ( 
        "cpuid;"
        : "+a"(out.eax), "=b"(out.ebx), "+c"(out.ecx), "=d"(out.edx)
    );

    return out;
}

static inline CoreID ThisID() {
    CPUIDResult model = CPUID(0, 0);

    bool isIntelLike = 
        model.ebx == 0x756E6547 &&
        model.edx == 0x49656E69 &&
        model.ecx == 0x6C65746E;

    if( isIntelLike ) return CPUID(0xB, 0).edx;
    
    u32 maxExtendedLeaf = CPUID(0x80000000, 0).eax;
    if( maxExtendedLeaf >= 0x8000001E )
        return CPUID(0x8000001E, 0).eax;

    return (CPUID(1, 0).ebx >> 24) & 0xFF;
}

static inline u64 ThisStorage() {
	u64 out;
	__asm__ __volatile__ (
        "movq %%gs:0, %0"
        : "=r"(out)
        :
        : "memory"
    );
	
	return out;
}

static inline void SwapStorage() {
	__asm__ __volatile__ ( "swapgs;" : : : "memory" );
}

static inline void SetStorage( bool active, void* storage ) {
	u32 ecx = active ? 0xC0000101 : 0xC0000102;
    u64 addr = (u64)storage;
	__asm__ __volatile__ ( 
		"wrmsr;" 
		:
		: "c"(ecx), "a"(addr & (0xFFFFFFFF)), "d"((addr >> 32))
		: "memory"
	);
}

static inline void IRQOff() {
    __asm__ __volatile__ ("cli");
}

static inline void IRQOn() {
    __asm__ __volatile__ ("sti");
}

static inline void KernelCrashUndBurn( void* messageAddr ) {
    __asm__ __volatile__ (
        "int3;"
        :
        : "a"(messageAddr), "b"(0xAABBCCDD)
        :
    );
    
    while(1);
    __builtin_unreachable();
}

static inline void LoadContext( void* ctx, ContextID id, bool flush ) {
	if( id >= (1 << PCID_BITS) ) KernelCrashUndBurn("PCID ID is huge.");
	
	u64 cr3 = id | (u64)ctx;
	if( !flush ) cr3 |= (1ULL << 63); // NO_FLUSH
	
    IRQOff();

	__asm__ __volatile__ (
		"mov %0, %%cr3;"
		:
		: "r"(cr3)
		: "memory"
	);

    IRQOn();
}


static inline void InvalidateContext( void* invAddr, ContextID id ) {
    if( id >= (1 << PCID_BITS) ) KernelCrashUndBurn("PCID ID is huge.");
	
	u64 pcidType = (invAddr == NULL) ? 0 : 1;
    u64 desc[2] = {id, (u64)invAddr};

    __asm__ __volatile__ (
        "invpcid %1, %0"
        :
        : "r"(pcidType), "m"(desc)
        : "memory"
    );
}


static inline void* ThisContext() {
    u64 out;
    __asm__ __volatile__ (
        "movq %%cr3, %0"
        : "=r"(out)
        :
        :
    );
    return (void*)((out >> PCID_BITS) << PCID_BITS);
}

static inline ContextID ThisContextID() {
    u64 out;
    __asm__ __volatile__ ( "movq %%cr3, %0" : "=r"(out) );
    return (ContextID)(out & ((1 << PCID_BITS) - 1));
}

static inline void FlushCachelines( Memory trg ) {
	usize base = AlignDown((usize)trg.Base, CACHELINE);
	for( usize i = 0; i < MemorySize(trg); i += CACHELINE ) {
		__asm__ __volatile__ (
			"clflushopt (%0)" 
			: 
			: "r" (base + i) 
			: "memory"
		);
	}
}

static inline void ArchitectureCheck() {
    CPUIDResult features = CPUID(0x01, 0x0);

    if( !(features.edx & (1 << 9)) ) KernelCrashUndBurn("Apparatus does not support APIC!");
    if( !(features.ecx & (1 << 17)) ) KernelCrashUndBurn("Apparatus does not support PCID!");
    if( !(features.ecx & (1 << 30)) ) KernelCrashUndBurn("Apparatus does not support RDRAND!");

    __asm__ __volatile__ ( // Enable PCID.
		"mov %%cr4, %%rax;"
		"bts $17, %%rax;"
		"mov %%rax, %%cr4;"
		:
		:
		: "rax", "flags", "memory"
	);


    __asm__ __volatile__ ( // Enable APIC.
		"mov $0x1B, %%ecx;"
        "rdmsr;"
        "bts $11, %%eax;"
        "wrmsr;"
		:
		:
		: "ecx", "edx", "eax", "memory"
	);


    __asm__ __volatile__ ( // Mask PIC interrupts
		"outb %0, $0x21;"
		"outb %0, $0xA1;"
        :
        : "a"((u8)0xFF)
	);
}
