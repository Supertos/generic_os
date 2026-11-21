/* Supertos Industries
	Generic OS UEFI Loader	
*/

#include <uefi.h>
#include <stdbool.h>

#include "ELF.h"
#include "Utils.h"
#include "Base.h"
#include "ACPI/Tables.h"

typedef void (*KernelEntry)( RootTable* );
