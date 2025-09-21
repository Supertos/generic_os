/* Supertos Industries
	Generic OS UEFI Loader utilities
*/
#include "utils.h"


/* LocateProtocol wrapper */
void* RequestProtocol( efi_guid_t protocol ) {
	void* outProtocol = NULL;
	
	BS->LocateProtocol( &protocol, NULL, &outProtocol );
	return outProtocol;
}


/* Basic (wasteful) space allocation */
AllocatedSpace AllocateBytes( size_t bytes ) {
	void* buffer = NULL;
	size_t pages = bytes / PAGE_SIZE + (bytes % PAGE_SIZE != 0);
	BS->AllocatePages( AllocateAnyPages, EfiLoaderData, pages, (size_t*)&buffer );
	
	return (AllocatedSpace){.Pages = pages, .Begin = buffer};
}


/* Basic (wasteful) space allocation with special type */
AllocatedSpace AllocateLoaderBytes( size_t bytes ) {
	void* buffer = NULL;
	size_t pages = bytes / PAGE_SIZE + (bytes % PAGE_SIZE != 0);
	BS->AllocatePages( AllocateAnyPages, EfiBootServicesData, pages, (size_t*)&buffer );
	
	return (AllocatedSpace){.Pages = pages, .Begin = buffer};
}


/* Downloads file to memory */
AllocatedSpace DownloadFileToMemory( wchar_t* path ) {
	efi_simple_file_system_protocol_t* simpleFS = RequestProtocol( (efi_guid_t)EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID );
	efi_guid_t fileInfoGUID = EFI_FILE_INFO_GUID;
	
	efi_file_handle_t* file;
	simpleFS->OpenVolume( simpleFS, &file );
	if( file->Open( file, &file, path, EFI_FILE_MODE_READ, 0 ) != EFI_SUCCESS ) return INVALID_ALLOCATION;
	
	size_t bufferSize = 0;
	file->GetInfo( file, &fileInfoGUID, &bufferSize, NULL );
	AllocatedSpace infoBuffer = AllocateLoaderBytes( bufferSize );
	
	file->GetInfo( file, &fileInfoGUID, &bufferSize, infoBuffer.Begin );
	size_t fileSize = ((efi_file_info_t*)(infoBuffer.Begin))->FileSize;
	BS->FreePages( (efi_physical_address_t)(infoBuffer.Begin), infoBuffer.Pages );
	
	AllocatedSpace fileBuffer = AllocateLoaderBytes( fileSize );
	size_t size = fileBuffer.Pages * PAGE_SIZE;
	file->Read( file, &size, fileBuffer.Begin );
	
	return fileBuffer;
}

