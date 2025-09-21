/* Supertos Industries
	Generic OS UEFI Loader
*/
#include "loader.h"


/* Form RootTable to pass to Kernel. The RootTable resides in EfiLoaderData while other data resides in EfiBootServicesData */
RootTable* FormRootTable() {
	size_t rootTableSize = sizeof(RootTable);
	
    size_t memoryMapSize = 0, descriptorSize = 0;
	BS->GetMemoryMap( &memoryMapSize, NULL, NULL, &descriptorSize, NULL );
	// Adding 6 because of +3 from RootTable and +3 from BS->GetMemoryMap in RecoverMemoryMap
	size_t memoryMapEntryCount = (memoryMapSize / descriptorSize) + 6; // Line 51 (Yeah, Passed MemoryMap is a bit fragmented)
	rootTableSize += memoryMapEntryCount * sizeof(MemoryEntry);
	
	AllocatedSpace rootTable = AllocateBytes( rootTableSize );
	
	RootTable* root = (RootTable*)(rootTable.Begin);
	
	root->MemoryMapEntryCount = memoryMapEntryCount;
	root->MemoryMap = (MemoryEntry*)((char*)(rootTable.Begin) + sizeof(RootTable));
	root->BlockSizeInPages = rootTable.Pages;
	
	return (RootTable*)(rootTable.Begin);
}


/* Returns true if specified memory type will be available for OS to allocate. */
/* EfiLoaderData is meant for data passed to OS */
bool IsAvailableMemoryType( size_t type ) {
	return type == EfiBootServicesCode ||
		   type == EfiLoaderCode ||
		   type == EfiBootServicesData ||
		   type == EfiConventionalMemory ||
		   type == EfiPersistentMemory ||
		   type == EfiUnacceptedMemoryType;
}


/* Recovers memory map from UEFI. Should be called after all kernel-related allocations done */
bool RecoverMemoryMap( RootTable* root ) {
    size_t memoryMapSize = 0, descriptorSize = 0;
	efi_status_t status = BS->GetMemoryMap( &memoryMapSize, NULL, NULL, &descriptorSize, NULL );
	if( status != EFI_BUFFER_TOO_SMALL ) return false;
	
	memoryMapSize += descriptorSize * 3; // Since allocation could split range into three ( FREE -> FREE + ALLOCATED + FREE )
	
	AllocatedSpace memMap = AllocateLoaderBytes( memoryMapSize );
	if( !memMap.Begin ) return false;
	
	BS->GetMemoryMap( &memoryMapSize, memMap.Begin, NULL, &descriptorSize, NULL ); 
	
	size_t count = memoryMapSize / descriptorSize;
	efi_memory_descriptor_t* descriptors = memMap.Begin;
	for( size_t i = 0; i < count; ++i ) {
		efi_memory_descriptor_t* d = (void*)((uint8_t*)descriptors + i * descriptorSize);

		root->MemoryMap[i] = (MemoryEntry){
			.Begin = (void*)(d->PhysicalStart),
			.Pages = d->NumberOfPages,
			.Type  = IsAvailableMemoryType(d->Type)
		};
	}
	
	BS->FreePages( (efi_physical_address_t)memMap.Begin, memMap.Pages );
    return true;
}


/* Recovers current video mode */
bool RecoverVideoMode( RootTable* root ) {
	efi_gop_t* gop = RequestProtocol( (efi_guid_t)EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID );
	if( !gop ) return false;
	
	root->Video = (VideoInfo){
		.Width = gop->Mode->Information->HorizontalResolution,
		.Height = gop->Mode->Information->VerticalResolution,
		.PixelsPerScanline = gop->Mode->Information->PixelsPerScanLine,
				
		.IsBGR = gop->Mode->Information->PixelFormat == PixelBlueGreenRedReserved8BitPerColor,
		.BufferBegin = (void*)gop->Mode->FrameBufferBase,
		.BufferSize = gop->Mode->FrameBufferSize
	};
	
	return true;
}


/* Recovers ACPI root table */
bool RecoverACPITable( RootTable* root ) {
	efi_guid_t acpiGUID = ACPI_20_TABLE_GUID;
	for( size_t id = 0; id < ST->NumberOfTableEntries; ++id ) {
		if( !memcmp(&ST->ConfigurationTable[id].VendorGuid, &acpiGUID, 16) ) {
			root->XSDP = ST->ConfigurationTable[id].VendorTable;
			
			if( memcmp( root->XSDP->Signature, "RSD PTR ", 8 ) ) return false;
			if( root->XSDP->Revision != 2 ) return false;
			
			uint8_t checksum = 0;
			for( size_t i = 0; i < XSDP_FIRST_PART_SIZE; ++i )
				checksum += ((uint8_t*)(root->XSDP))[i];
			
			if( checksum ) return false
			
			for( size_t i = 0; i < sizeof(struct XSDP_t) - XSDP_FIRST_PART_SIZE; ++i )
				checksum += ((uint8_t*)(root->XSDP))[i];
			
			if( checksum ) return false
			
			
			return true;
		}
	}
	
	return false;
}


/* Entry point */
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
	
	AllocatedSpace kernel = DownloadFileToMemory( L"\\ELF\\CORE\\KERNEL.ELF" );
	if( !kernel.Begin || !ValidateELF( kernel.Begin ) ) return 1;
	
	ELFInfo kernelInfo = RetrieveELFInfo( kernel.Begin );
	
	AllocatedSpace kernelTarget = AllocateBytes( kernelInfo.executiveSize );
	if( !kernelTarget.Begin ) return 2;
	
	kernelentry_t KernelEntry = (kernelentry_t)(uintptr_t)LoadELF( kernel.Begin, kernelTarget.Begin, &kernelInfo );
	
	// TODO: FIXME: The order is swapped (We load Kernel first and only afterwards create RootTable)
	// This is because when loading .ELF we split UEFI memory map and shouldn't rely on any data we've recovered back then
	// FormRootTable accounts for it's own allocation and all allocations afterwards (Yeah, by adding +3 with every following allocation)
	
	AllocatedSpace stackPages = AllocateBytes( PAGE_SIZE * 16 );
	
	
	
	RootTable* root = FormRootTable();
	
	if( !RecoverVideoMode(root) ) return 1;
	if( !RecoverACPITable(root) ) return 2;
	if( !RecoverMemoryMap(root) ) return 3;

	exit_bs(); // posix-uefi does MemKey for us
	KernelEntry( root );
	
	__asm__ __volatile__ (
        "mov %[stack], %%rsp;"     
        "mov %[arg], %%rdi;"       
        "call *%[entry];"          
        "ud2;"                     
        :
        : [stack] "r" (stackPages.Begin),
          [entry] "r" (KernelEntry),
          [arg]   "r" (root)
        : "rsp", "rdi"
    );
}