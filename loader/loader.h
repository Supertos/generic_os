/* Supertos Industries
	Generic OS UEFI Loader
*/
#include <uefi.h>
#include <stdbool.h>

#include "elf.h"
#include "utils.h"

#define PAGE_SIZE 4096
#define XSDP_FIRST_PART_SIZE 8

typedef struct VideoInfo VideoInfo;
struct VideoInfo {
	void* BufferBegin;
	size_t BufferSize;
	size_t Width;
	size_t Height;
	size_t PixelsPerScanline;
	bool IsBGR;
};


typedef struct MemoryEntry MemoryEntry;
struct MemoryEntry {
	void* Begin;
	size_t Pages : 56;
	uint8_t Type : 8;
} __attribute__((__packed__));


struct XSDP_t {
 char Signature[8];
 uint8_t Checksum;
 char OEMID[6];
 uint8_t Revision;
 uint32_t RsdtAddress;

 uint32_t Length;
 uint64_t XsdtAddress;
 uint8_t ExtendedChecksum;
 uint8_t reserved[3];
} __attribute__ ((packed));

typedef struct RootTable RootTable;
struct RootTable {
	struct XSDP_t* XSDP;
	size_t MemoryMapEntryCount;
	MemoryEntry* MemoryMap;
	VideoInfo Video;
	size_t BlockSizeInPages;
};

typedef void (*kernelentry_t)( RootTable* );
