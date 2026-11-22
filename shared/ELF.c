/* Supertos Industries (2012 - 2026) */
#include "ELF.h"

#define ELF_PROGRAM(elf, id) (ELFProgram*)((char*)elf + elf->ProgramHeaderOffset + elf->ProgramHeaderSize * id)
#define ELF_SECTION(elf, id) (ELFSection*)((char*)elf + elf->SectionHeaderOffset + elf->SectionHeaderSize * id)


static u8 VALID_ELF_IDENT[] = {
	0x7F, 'E', 'L', 'F',
	ELFCLASS64,
	ELFDATA2LSB,
	EV_CURRENT,
	ELFOSABI_SYSV,
	0
};


bool ELFValid( may_alias ELF* self ) {
	if( self->Type != ET_DYN ) return false;
	
	may_alias u8* elfIdent = (may_alias u8*)self;
	for( usize i = 0; i < sizeof(VALID_ELF_IDENT); i++ )
		if( VALID_ELF_IDENT[i] != elfIdent[i] ) return false;

	return true;
}


#define R_X86_64_NONE      0
#define R_X86_64_64        1 
#define R_X86_64_PC32      2
#define R_X86_64_GLOB_DAT  6
#define R_X86_64_JUMP_SLOT 7
#define R_X86_64_RELATIVE  8

#define DT_NULL 		   0
#define DT_STRTAB 		   5
#define DT_SYMTAB 		   6
#define DT_RELA 		   7
#define DT_RELASZ 		   8
#define DT_RELAENT 		   9
#define DT_STRSZ 		   10
#define DT_SYMENT 		   11

#define SHT_DYNSYM		   11

#define SHN_UNDEF		   0

#define ELF32_ST_BIND(i) ((i)>>4)
#define ELF32_ST_TYPE(i) ((i)&0xf)


static inline void ProcessDynamic( const ELFProgram* self, const ELF* elf, ELFInfo* info ) {
	char* 		 base    = (char*)elf;
	ELFDynEntry* entries = (ELFDynEntry*)( base + self->DataOffset );

	for( usize i = 0; entries[i].Type != DT_NULL; i++ ) {
		ELFDynEntry cur = entries[i];

		if( cur.Type == DT_RELA ) info->RELABase = (ELFRelocation*)(base + cur.Value);
		if( cur.Type == DT_SYMTAB ) info->SYMBase = (ELFSymbol*)(base + cur.Value);
		if( cur.Type == DT_STRTAB ) info->STRBase = base + cur.Value;
		
		if( cur.Type == DT_RELASZ ) info->RELASize = cur.Value;
		if( cur.Type == DT_RELAENT ) info->RELAEntrySize = cur.Value;
		if( cur.Type == DT_STRSZ ) info->STRSize = cur.Value;
		if( cur.Type == DT_SYMENT ) info->SYMEntrySize = cur.Value;
	}

	info->SYMEntrySize  = info->SYMEntrySize  ?: sizeof(ELFSymbol);
	info->RELAEntrySize = info->RELAEntrySize ?: sizeof(ELFRelocation);
}


ELFInfo RetrieveELFInfo( const ELF* self ) {
	ELFInfo out = {0};
	out.Range = (Memory){ ~0ULL, 0 };

	for( usize i = 0; i < self->ProgramHeaderCount; i++ ) {
		ELFProgram* cur = ELF_PROGRAM(self, i);
		
		if( cur->Type == PT_LOAD ) {
			out.Range.End  = max( out.Range.End,  cur->VAddr + cur->MemorySize );
			out.Range.Base = min( out.Range.Base, cur->VAddr );
		}else if( cur->Type == PT_DYNAMIC ) {
			ProcessDynamic(cur, self, &out);
		}
	}

	for( usize i = 0; i < self->SectionHeaderCount; i++ ) {
		ELFSection* cur = ELF_SECTION(self, i);

		if( cur->Type == SHT_DYNSYM ) {
			out.DYNSYMSection = cur;
		}
	}

	return out;
}


void* LoadELF( const ELF* self, const Memory trg, const ELFInfo info ) {
	u64 loadBias = trg.Base - info.Range.Base;
	
	for( usize i = 0; i < self->ProgramHeaderCount; i++ ) {
		ELFProgram* cur = ELF_PROGRAM(self, i);
		if( cur->Type != PT_LOAD ) continue;

		Memory target = (Memory){loadBias + cur->VAddr, trg.End};
		Memory source = MemoryNew((char*)self + cur->DataOffset, cur->FileSize);
		MemoryCopy(source, target);
	}

	return (void*)(loadBias + self->Entry);
}


ExternalSymbol* NextSymbol( void* tbl ) {
	ExternalSymbol* cur = (ExternalSymbol*)tbl;
	cur = (ExternalSymbol*)((char*)cur + cur->Length);
	return cur->Length != 0 ? cur : NULL;
}


u64 ResolveSymbol( ELFRelocation self, u64 loadBias, ELFInfo* info, void* importTable ) {
	if( self.Index == 0 ) return CANT_RESOLVE;
	ELFSymbol* symbol = (ELFSymbol*)((char*)info->SYMBase + info->SYMEntrySize * self.Index);
	if( symbol->Shndx != SHN_UNDEF ) return loadBias + symbol->Value;

	ExternalSymbol* cur = (ExternalSymbol*)importTable;
	while( cur != NULL ) {
		if( StringCompare(cur->Name, info->STRBase + symbol->Name) )
			return cur->Value;

		cur = NextSymbol(cur);
	}

	return CANT_RESOLVE;
}


usize RelocateELF( Memory trg, ELFInfo info, void* importTable ) {
	usize unresolvedSymbols = 0;
	u64   loadBias    		= trg.Base - info.Range.Base;
	usize relaCount 		= info.RELASize / info.RELAEntrySize;

	for( usize i = 0; i < relaCount; i++ ) {
		usize 		   offset = info.RELAEntrySize * i;
		ELFRelocation  cur    = *(ELFRelocation*)((char*)info.RELABase + offset);

		may_alias u64* u64Trg = (u64*)(loadBias + cur.Offset);
		may_alias i32* i32Trg = (i32*)(loadBias + cur.Offset);

		if( cur.Type == R_X86_64_RELATIVE ) {
			*u64Trg = loadBias + cur.Addend;
			continue;
		}

		
		u64 symbol = ResolveSymbol(cur, loadBias, &info, importTable);
		if( symbol == CANT_RESOLVE ) {
			unresolvedSymbols++;
			continue;
		}

		switch( cur.Type ) {
			case R_X86_64_GLOB_DAT:
			case R_X86_64_JUMP_SLOT:
				*u64Trg = symbol;
				break;
			case R_X86_64_64:
				*u64Trg = symbol + cur.Addend;
				break;
			case R_X86_64_PC32:
				*i32Trg = (i32)((i64)symbol + (i64)cur.Addend - (i64)i32Trg);
				break;
			default:
				continue;
		}
	}
	return unresolvedSymbols;
}

u64 ExportTableSize( ELFInfo info ) {
	return info.STRSize + (info.DYNSYMSection->Size / info.DYNSYMSection->EntrySize + 1) * sizeof(ExternalSymbol);
}

void ExtractELFSymbols( ELF* self, ELFInfo info, Memory loadedELF, Memory tbl ) {
	u64   loadBias      = loadedELF.Base - info.Range.Base;
	ExternalSymbol* cur = (ExternalSymbol*)(tbl.Base);

	usize symCount = info.DYNSYMSection->Size / info.DYNSYMSection->EntrySize;

	ELFSymbol* entries = (ELFSymbol*)((char*)self + info.DYNSYMSection->Offset);
	for( usize i = 1; i < symCount; i++ ) {
		if( entries[i].Shndx == SHN_UNDEF ) continue;

		cur->Length = sizeof(ExternalSymbol) + StringCopy(info.STRBase + entries[i].Name, cur->Name);
		cur->Value = loadBias + entries[i].Value;

		cur = NextSymbol(cur);
	}

	*cur = (ExternalSymbol){0};
}