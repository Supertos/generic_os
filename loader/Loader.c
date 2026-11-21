/* Supertos Industries (2012 - 2026) */
#include "Loader.h"

static_assert( sizeof(MemoryEntry) < sizeof(efi_memory_descriptor_t) );

static inline RootTable* RootTableNew( usize* descSize ) {
	usize mapSize   = 0;
	usize descCount = 0;
	usize rootSize  = AlignUp( sizeof(RootTable), alignof(void*) );
	
	efi_status_t status = BS->GetMemoryMap( &mapSize, NULL, NULL, descSize, NULL );
	if( status != EFI_BUFFER_TOO_SMALL ) return NULL;

	descCount = (mapSize / *descSize) + 3; // Further allocation could split range into 3 (FREE + ALLOCATED + FREE)

	Memory root = AllocateBytes( rootSize + descCount * *descSize );
	
	may_alias RootTable* self = (may_alias RootTable*)root.Base;
	*self = (RootTable){
		.MemMapCount = descCount,
		.MemoryMap   = (may_alias MemoryEntry*)((may_alias u8*)self + rootSize)
	};

	status = BS->GetMemoryMap( &mapSize, (void*)self->MemoryMap, NULL, descSize, NULL );
	if( status != EFI_SUCCESS ) {
		FreeBytes(root);
		return false;
	}
	return self;
}

bool MemoryAvailable( usize type ) {
	return /* type == EfiBootServicesCode || */
		   /* type == EfiLoaderCode || */
		   /* type == EfiBootServicesData || */
		   type == EfiConventionalMemory ||
		   type == EfiPersistentMemory ||
		   type == EfiUnacceptedMemoryType;
}


/* Recovers memory map from UEFI. Should be called after all kernel-related allocations done */
bool RecoverMemoryMap( RootTable* root, usize descSize ) {

	for( u64 i = 0; i < root->MemMapCount; i++ ) {
		MemoryEntry new;

		{
			efi_memory_descriptor_t cur;
			u8* mapBase = (u8*)root->MemoryMap;
			memcpy( &cur, mapBase + descSize * i, sizeof(cur) );
			
			Memory range = MemoryNew( cur.PhysicalStart, cur.NumberOfPages * PAGE );

			u64    type  = (MemoryAvailable(cur.Type) ? MEMMAP_AVALIABLE : 0) |
						   (((u64)cur.VirtualStart == (u64)root) ? MEMMAP_ROOTTABLE : 0);

			new = (MemoryEntry){ range, type };	
		}

		root->MemoryMap[i] = new;
	}

	return true;
}


/* Recovers current video mode */
bool RecoverVideoMode( RootTable* root ) {
	efi_gop_t* gop = RequestProtocol( (efi_guid_t)EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID );
	if( !gop ) return false;

	efi_gop_mode_t 		mode = *gop->Mode;
	efi_gop_mode_info_t info = *mode.Information;
	
	root->Video = (VideoInfo){
		.BGR 		   = (info.PixelFormat == PixelBlueGreenRedReserved8BitPerColor),
		.Buffer 	   = (void*)mode.FrameBufferBase,
		.Height 	   = info.VerticalResolution,
		.Width  	   = info.HorizontalResolution,
		.ScanlineWidth = info.PixelsPerScanLine
	};
	
	return true;
}


/* Recovers ACPI root table */
bool RecoverACPITable( RootTable* root ) {
	efi_guid_t guid = ACPI_20_TABLE_GUID;

	for( usize id = 0; id < ST->NumberOfTableEntries; ++id ) {
		efi_configuration_table_t* cur = &ST->ConfigurationTable[id];
		if( memcmp(&cur->VendorGuid, &guid, sizeof(guid)) ) continue;
		
		XSDP* xsdp = (XSDP*)cur->VendorTable;
		if( memcmp( &xsdp->Signature, "RSD PTR ", 8 ) || xsdp->Revision != 2 ) // You shall do checksum of the first 20 bytes before checking Revision number.
			return false;

		u8 checksum = 0;
		for( usize i = 0; i < sizeof(XSDP); ++i )
			checksum += ((u8*)(xsdp))[i];

		if( checksum != 0 ) return false;

		root->XSDT = xsdp->XSDT;
		return true;
	}

	return false;
}


__attribute__((noinline)) void SwitchStackAndJump(KernelEntry entry, void* arg, u64 stack) {
    __asm__ __volatile__ (
        "mov %[stack], %%rsp;"
		"andq $-16, %%rsp;"
        "mov %[arg], %%rdi;"
        "jmp *%[entry];"
        :
        : [stack] "r" (stack), [entry] "r" (entry), [arg] "r" (arg)
        : "rdi", "memory"
    );
    __builtin_unreachable();
}


/* Entry point */
int main() {
	asm volatile("andq $-16, %%rsp" : : : ); 
	Memory image = DownloadFileToMemory( L"\\ELF\\CORE\\KERNEL.ELF" );
	if( !MemoryValid(image) || !ELFValid((ELF*)image.Base) ) return 1;
	ELFInfo info = RetrieveELFInfo((ELF*)image.Base);

	Memory target = AllocateBytes(info.size);
	if( !MemoryValid(target) ) {
		FreeBytes(image);
		return 1;
	}
	
	KernelEntry entry = (KernelEntry)LoadELF( (ELF*)image.Base, (void*)target.Base, info );
	RelocateELF((ELF*)image.Base, (void*)target.Base, info);
	
	Memory stack = AllocateBytes(PAGE * 128);

	usize memDescSize = 0;
	RootTable* root = RootTableNew(&memDescSize);
	
	if( !RecoverVideoMode(root) || !RecoverACPITable(root) || !RecoverMemoryMap(root, memDescSize) )
		return 1;

	printf("Running \\ELF\\CORE\\KERNEL.ELF...\n");
	exit_bs(); // posix-uefi does MemKey for us
	SwitchStackAndJump(entry, root, stack.End);
}