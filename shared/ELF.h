/* Supertos Industries
	Generic OS UEFI Loader
*/
#pragma once

#include "Base.h"

#define ELFCLASS64 2

#define ELFDATA2LSB 1

#define ELFOSABI_SYSV 0

#define EV_CURRENT 1

#define ET_DYN 3

#define PT_NULL 0
#define PT_LOAD 1
#define PT_DYNAMIC 2

typedef struct {
	u32 Type;
	u32 Flags;
	u64 DataOffset;
	u64 VAddr;
	u64 PAddr;
	u64 FileSize;
	u64 MemorySize;
	u64 Alignment;
} ELFProgram;

typedef struct {
	u8  Identity[16];
	u16 Type;
	u16 Machine;
	u32 Version;
	u64 Entry;
	u64 ProgramHeaderOffset;
	u64 SectionHeaderOffset;
	u32 CPUFlags;
	u16 HeaderSize;
	u16 ProgramHeaderSize;
	u16 ProgramHeaderCount;
	u16 SectionHeaderSize;
	u16 SectionHeaderCount;
	u16 SectionNameIndex;
} ELF;

typedef struct {
	u32 Name;
	u32 Type;
	u64 Flags;
	u64 VAddr;
	u64 Offset;
	u64 Size;
	u32 Link;
	u32 Info;
	u64 Align;
	u64 EntrySize;
} ELFSection;

typedef struct {
    i64 Type;
    u64 Value;
} ELFDynEntry;

typedef struct {
    u64 Offset;
    u32 Type;
    u32 Index;
    i64 Addend;
} ELFRelocation;

typedef struct {
    u32 Name;
    u8  Info;
    u8  Other;
    u16 Shndx;
    u64 Value;
    u64 Size; 
} ELFSymbol;

typedef struct {
	Memory 			Range;
	ELFRelocation*  RELABase;
	ELFSymbol*  	SYMBase;
	char*  			STRBase;
	ELFSection*		DYNSYMSection;
	u64    			RELASize;
	u64    			STRSize;
	u64    			RELAEntrySize;
	u64    			SYMEntrySize;
} ELFInfo;

#define CANT_RESOLVE 0

struct ExternalSymbol {
	u64 Length;
	u64 Value;
	char Name[]; 
};

void* LoadELF( const ELF* self, const Memory trg, const ELFInfo info );

ELFInfo RetrieveELFInfo( const ELF* elf );

bool ELFValid( ELF* header );

usize RelocateELF( Memory trg, ELFInfo info, void* importTable );

u64 ExportTableSize( ELFInfo info );

void ExtractELFSymbols( ELF* self, ELFInfo info, Memory loadedELF, Memory tbl );

ExternalSymbol* NextSymbol( void* tbl );

u64 ResolveSymbol( ELFRelocation self, u64 loadBias, ELFInfo* info, void* importTable );