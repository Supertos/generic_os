/* Supertos Industries
	Generic OS UEFI Loader
*/
#include "elf.h"

// #define max( a, b ) (a > b ? a : b)

unsigned char VALID_ELF_IDENT[] = {
	0x7F, 'E', 'L', 'F',
	ELFCLASS64,
	ELFDATA2LSB,
	EV_CURRENT,
	ELFOSABI_SYSV,
	0
};
static inline bool IsValidELF( ELFHeader* header ) {
	return !memcmp( header->identity, VALID_ELF_IDENT, sizeof(VALID_ELF_IDENT) );
}
 
static inline bool IsDYNELF( ELFHeader* header ) {
	return header->type == ET_DYN;
}

bool ValidateELF( ELFHeader* elf ) {
	if( IsValidELF( elf ) && IsDYNELF( elf ) ) return true;
	
	printf( "Invalid .ELF file passed at %016x.\n", elf );
	printf( "%8s|%8s\n", "PROVIDED", "VALID" );
	
	printf( "%4d %c%c%c|%4d %c%c%c\n", elf->identity[0], elf->identity[1], elf->identity[2], elf->identity[3], 
									   VALID_ELF_IDENT[0], VALID_ELF_IDENT[1], VALID_ELF_IDENT[2], VALID_ELF_IDENT[3] );
	
	
	
	printf( "%1u%1u%1u%1u%1u   |   %1u%1u%1u%1u%1u\n", elf->identity[4], elf->identity[5], 
													   elf->identity[6], elf->identity[7], elf->identity[8],
													   VALID_ELF_IDENT[4], VALID_ELF_IDENT[5],
													   VALID_ELF_IDENT[6], VALID_ELF_IDENT[7], VALID_ELF_IDENT[8] );
	
	printf( "Type: %2d|Type: %2d\n", elf->type, ET_DYN );
	
	return false;
}


/* Returns required memory space to load ELF (Validation is upon caller) */
ELFInfo RetrieveELFInfo( const ELFHeader* elf ) {
	ELFProgramHeader* program = (ELFProgramHeader*)((char*)elf + elf->programHeaderOffset);	
	uint32_t headersLeft = elf->programHeaderCount;
	uint64_t highestMem = 0, lowestMem = ~0;

	do {
		if( program->type == PT_LOAD ) {
			highestMem = max( highestMem, program->vAddr + program->sizeInMemory );
			lowestMem = min( lowestMem, program->vAddr );
		}
		program = (ELFProgramHeader*)((char*)program + elf->programHeaderSize);
	}while( --headersLeft > 0 );
	
	return (ELFInfo){.executiveSize = highestMem - lowestMem, .executiveBase = lowestMem};
}

/* Loads PT_LOAD segments of PIE ELF into caller allocated buffer trg. (Validation is upon caller) */
void* LoadELF( ELFHeader* elf, void* trg, ELFInfo* info ) {
	uint64_t base = info->executiveBase;
	
	ELFProgramHeader* program = (ELFProgramHeader*)((char*)elf + elf->programHeaderOffset);	
	uint32_t headersLeft = elf->programHeaderCount;
	
	do{
		if( program->type == PT_LOAD ) {
			char* loadAddr = (char*)trg + program->vAddr - base;
			if( program->sizeInMemory > program->sizeInFile ) 
				memset( loadAddr + program->sizeInFile, 0, program->sizeInMemory - program->sizeInFile );
			memcpy( loadAddr, (char*)elf + program->dataOffset, program->sizeInFile );
		}
		program = (ELFProgramHeader*)((char*)program + elf->programHeaderSize);
	}while( --headersLeft > 0 );
	
	return (void*)((char*)trg + elf->entry - base);
}

