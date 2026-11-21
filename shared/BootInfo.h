/* Supertos Industries (2012 - 2026) 
 * 
 */
#pragma once

#include "Base.h"

typedef struct VideoInfo VideoInfo;
struct VideoInfo {
	u32* Buffer;
	u16 Width;
	u16 Height;
	u16 ScanlineWidth;
	bool BGR;
};

typedef struct MemoryEntry MemoryEntry;
struct MemoryEntry {
	Memory Range;
	u64 Type;
};

typedef struct RootTable RootTable;
struct RootTable {
	void* XSDT;
	u64 MemMapCount;
	may_alias MemoryEntry* MemoryMap;
	VideoInfo Video;
	Memory ELFLoadedRange;
};

#define MEMMAP_AVALIABLE 0b01ULL
#define MEMMAP_RECLAIMABLE 0b010ULL
#define MEMMAP_ROOTTABLE 0b0100ULL

#if defined(__x86_64__)

#else
    #error "Unknown table provider."
#endif
