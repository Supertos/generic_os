/* Supertos Industries
	Generic OS UEFI Loader
*/
#include <uefi.h>
#include <stdbool.h>

#define PAGE_SIZE 4096

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
	uint32_t type;
	uint32_t flags;
	uint64_t dataOffset;
	uint64_t vAddr;
	uint64_t pAddr;
	uint64_t sizeInFile;
	uint64_t sizeInMemory;
	uint64_t alignment;
} ELFProgramHeader;

typedef struct ELFHeader {
	uint8_t  identity[16];
	uint16_t type;
	uint16_t machine;
	uint32_t version;
	uint64_t entry;
	uint64_t programHeaderOffset;
	uint64_t sectionHeaderOffset;
	uint32_t cpuFlags;
	uint16_t headerSize;
	uint16_t programHeaderSize;
	uint16_t programHeaderCount;
	uint16_t sectionHeaderSize;
	uint16_t sectionHeaderCount;
	uint16_t sectionNameIndex;
} ELFHeader;

typedef struct ELFInfo {
	uint64_t executiveSize;
	uint64_t executiveBase;
} ELFInfo;

void* LoadELF( ELFHeader* begin, void* trg, ELFInfo* info );
ELFInfo RetrieveELFInfo( const ELFHeader* elf );
bool ValidateELF( ELFHeader* header );
