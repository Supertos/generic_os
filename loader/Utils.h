/* Supertos Industries
	Generic OS UEFI Loader
*/
#define GENERICBOOTLOADER 1

#include <uefi.h>
#include "Base.h"


static inline void* RequestProtocol( efi_guid_t protocol ) {
	void* outProtocol = NULL;
	
	BS->LocateProtocol( &protocol, NULL, &outProtocol );
	return outProtocol;
}


static inline Memory AllocateType( usize bytes, efi_memory_type_t type ) {
	usize base;
	usize pages = AlignUp(bytes, PAGE) / PAGE;
	BS->AllocatePages( AllocateAnyPages, type, pages, &base );
	return MemoryNew(base, pages * PAGE);
}


static inline Memory AllocateBytes( usize bytes ) {
	return AllocateType(bytes, EfiLoaderData);
}


static inline Memory AllocateLoaderBytes( usize bytes ) {
	return AllocateType(bytes, EfiBootServicesData);
}


static inline void FreeBytes( Memory in ) {
	BS->FreePages( in.Base, MemorySize(in) / PAGE );
}


static inline Memory DownloadFileToMemory( wchar_t* path ) {
	usize  bufSize  = 0;
	Memory buffer   = MemoryInvalid;

	efi_simple_file_system_protocol_t* fs = RequestProtocol( (efi_guid_t)EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID );
	efi_guid_t fileInfoGUID = EFI_FILE_INFO_GUID;

	efi_file_handle_t* file;
	fs->OpenVolume(fs, &file);
	efi_status_t fileStatus = file->Open( file, &file, path, EFI_FILE_MODE_READ, 0 );
	if( fileStatus != EFI_SUCCESS ) return MemoryInvalid;
	
	file->GetInfo( file, &fileInfoGUID, &bufSize, NULL );
	buffer = AllocateLoaderBytes(bufSize);
	
	file->GetInfo( file, &fileInfoGUID, &bufSize, (void*)buffer.Base );

	bufSize = ((efi_file_info_t*)(buffer.Base))->FileSize;
	FreeBytes(buffer);
	
	buffer = AllocateLoaderBytes(bufSize);
	file->Read( file, &bufSize, (void*)buffer.Base );
	return buffer;
}

