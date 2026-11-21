/* Supertos Industries
	Generic OS UEFI Loader
*/

#include <uefi.h>

#include "Base.h"

enum ELF_CLASS {
	ELFCLASS32 = 1,
	ELFCLASS64 = 2
};

enum ELF_ENCODING {
	ELFDATA2LSB = 1,
	ELFDATA2MSB = 2
};

enum ELF_OSABI {
	ELFOSABI_SYSV = 0,
	ELFOSABI_HPUX = 1,
	ELFOSABI_STANDALONE = 255
};

enum ELF_VER {
	EV_CURRENT = 1
};

enum ELF_TYPE {
	ET_NONE = 0,
	ET_REL = 1,
	ET_EXEC = 2,
	ET_DYN = 3,
	ET_CORE = 4
};

enum ELFSEG_TYPE {
	PT_NULL,
	PT_LOAD,
	PT_DYNAMIC,
	PT_INTERP,
	PT_NOTE,
	PT_SHLIB,
	PT_PHDR
};

typedef struct ELFProgramHeader {
	u32 type;
	u32 flags;
	u64 dataOffset;
	u64 vAddr;
	u64 pAddr;
	u64 sizeInFile;
	u64 sizeInMemory;
	u64 alignment;
} ELFProgramHeader;

typedef struct ELF {
	u8  identity[16];
	u16 type;
	u16 machine;
	u32 version;
	u64 entry;
	u64 programHeaderOffset;
	u64 sectionHeaderOffset;
	u32 cpuFlags;
	u16 headerSize;
	u16 programHeaderSize;
	u16 programHeaderCount;
	u16 sectionHeaderSize;
	u16 sectionHeaderCount;
	u16 sectionNameIndex;
} ELF;

typedef struct ELFInfo {
	u64 size;
	u64 base;
} ELFInfo;


typedef struct ELFDynEntry {
    i64 Type;
    u64 Value;
} ELFDynEntry;


typedef struct ELFRelaEntry {
    u64 Offset;
    u32 Type;
    u32 Index;
    i64 Addend;
} ELFRelaEntry;


typedef struct {
    u32 Name;
    u8  Info;
    u8  Other;
    u16 Shndx;
    u64 Value;
    u64 Size; 
} ELFSymEntry;


void* LoadELF( ELF* begin, void* trg, ELFInfo info );

ELFInfo RetrieveELFInfo( const ELF* elf );

bool ELFValid( ELF* header );

void RelocateELF( ELF* elf, void* loadAddr, ELFInfo info );