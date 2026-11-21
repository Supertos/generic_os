/* Supertos Industries (2012 - 2026) */
#include "ELF.h"


unsigned char VALID_ELF_IDENT[] = {
	0x7F, 'E', 'L', 'F',
	ELFCLASS64,
	ELFDATA2LSB,
	EV_CURRENT,
	ELFOSABI_SYSV,
	0
};


bool ELFValid( ELF* elf ) {
	if( elf->type == ET_DYN && !memcmp( elf->identity, VALID_ELF_IDENT, sizeof(VALID_ELF_IDENT) ) ) return true;
	
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
ELFInfo RetrieveELFInfo( const ELF* elf ) {
	ELFProgramHeader* program = (ELFProgramHeader*)((char*)elf + elf->programHeaderOffset);	
	u32 headersLeft = elf->programHeaderCount;
	u64 highestMem = 0, lowestMem = ~0;

	do {
		if( program->type == PT_LOAD || program->type == PT_DYNAMIC ) {
			highestMem = max( highestMem, program->vAddr + program->sizeInMemory );
			lowestMem = min( lowestMem, program->vAddr );
		}
		program = (ELFProgramHeader*)((char*)program + elf->programHeaderSize);
	}while( --headersLeft > 0 );
	
	return (ELFInfo){.size = highestMem - lowestMem, .base = lowestMem};
}

/* Loads PT_LOAD segments of PIС ELF into caller allocated buffer trg. (Validation is upon caller) */
void* LoadELF( ELF* elf, void* trg, ELFInfo info ) {
	u64 base = info.base;
	
	ELFProgramHeader* program = (ELFProgramHeader*)((char*)elf + elf->programHeaderOffset);	
	u32 headersLeft = elf->programHeaderCount;
	
	do{
		if( program->type == PT_LOAD ) {
			char* loadAddr = (char*)trg + program->vAddr - base;
			printf("Loaded PT_LOAD at: %p\n", loadAddr);
			if( program->sizeInMemory > program->sizeInFile ) 
				memset( loadAddr + program->sizeInFile, 0, program->sizeInMemory - program->sizeInFile );
			memcpy( loadAddr, (char*)elf + program->dataOffset, program->sizeInFile );
		}
		program = (ELFProgramHeader*)((char*)program + elf->programHeaderSize);
	}while( --headersLeft > 0 );
	
	return (void*)((char*)trg + elf->entry - base);
}

#define R_X86_64_NONE      0
#define R_X86_64_64        1 
#define R_X86_64_PC32      2
#define R_X86_64_GLOB_DAT  6
#define R_X86_64_JUMP_SLOT 7
#define R_X86_64_RELATIVE  8

#define DT_STRTAB 		   5
#define DT_SYMTAB 		   6
#define DT_RELA 		   7
#define DT_RELASZ 		   8
#define DT_RELAENT 		   9

#define ELF32_ST_BIND(i) ((i)>>4)
#define ELF32_ST_TYPE(i) ((i)&0xf)
#define ELF32_ST_INFO(b,t) (((b)<<4)+((t)&0xf))


/* Applies relocations based on elf contents to loaded at loadAddr PT_LOADs */
void RelocateELF( ELF* elf, void* loadAddr, ELFInfo info ) {
	u64 base = (u64)loadAddr - info.base, elfBase = (u64)elf;
	
	ELFProgramHeader* program = (ELFProgramHeader*)((char*)elf + elf->programHeaderOffset);	
	u32 headersLeft = elf->programHeaderCount;
	
	do{
		if( program->type == PT_DYNAMIC ) {
			printf("Relocation applied!\n");
			ELFDynEntry* entry = (ELFDynEntry*)((char*)elf + program->dataOffset);

			u64 relaTable = (u64)NULL;
			u64 relaSize = 0;
			u64 entrySize = 0;
			u64 symTable = (u64)NULL;
			u64 strTable = (u64)NULL;

			for( usize i = 0; entry[i].Type != 0; i++ ) {
				if( entry[i].Type == DT_RELA ) relaTable = elfBase + entry[i].Value;
				if( entry[i].Type == DT_RELAENT ) entrySize = entry[i].Value;
				if( entry[i].Type == DT_RELASZ ) relaSize = entry[i].Value;
				if( entry[i].Type == DT_SYMTAB ) symTable = elfBase + entry[i].Value;
				if( entry[i].Type == DT_STRTAB ) strTable = elfBase + entry[i].Value;

				if( entry[i].Type < DT_STRTAB || entry[i].Type > DT_RELAENT ) printf("Invalid DynEntry type %d!\n", entry[i].Type);
			}

			entrySize = entrySize ?: sizeof(ELFRelaEntry);
			u64 relaCount = relaSize / entrySize;

			ELFSymEntry* syms = (ELFSymEntry*)symTable;

			for( u64 i = 0; i < relaCount; i++ ) {
				ELFRelaEntry* entry = &(((ELFRelaEntry*)relaTable)[i]);

				u64 type = entry->Type;
				u64 idx = entry->Index;

				u64* trg = (u64*)(base + entry->Offset);

				u64 symVal = idx > 0 ? (base + syms[idx].Value) : 0;

				if( type == R_X86_64_RELATIVE ) *trg = base + entry->Addend;
				if( type == R_X86_64_64 ) *trg = symVal + entry->Addend;
				if( type == R_X86_64_GLOB_DAT ) *trg = symVal;
				if( type == R_X86_64_JUMP_SLOT ) *trg = symVal;
				if( type == R_X86_64_PC32 ) {
					i64 delta = (i64)symVal + entry->Addend - (i64)trg;
					*(i32*)trg = (i32)delta;
				}
			}
			
		}
		program = (ELFProgramHeader*)((char*)program + elf->programHeaderSize);
	}while( --headersLeft > 0 );
}