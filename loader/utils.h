/* Supertos Industries
	Generic OS UEFI Loader
*/
#include <uefi.h>

#define PAGE_SIZE 4096
#define INVALID_ALLOCATION (AllocatedSpace){.Begin = NULL, .Pages = 0}

typedef struct AllocatedSpace AllocatedSpace;
struct AllocatedSpace {
	void* Begin;
	size_t Pages;
};


/* LocateProtocol wrapper */
void* RequestProtocol( efi_guid_t protocol );


/* Basic (wasteful) space allocation */
AllocatedSpace AllocateBytes( size_t bytes );


/* Basic (wasteful) space allocation with special type */
AllocatedSpace AllocateLoaderBytes( size_t bytes );


/* Downloads file to memory */
AllocatedSpace DownloadFileToMemory( wchar_t* path );